use std::fs::File;
use std::io::{Read, BufReader};
use std::convert::TryInto;
use std::time::Duration;

use rdkafka::config::ClientConfig;
use rdkafka::producer::{FutureProducer, FutureRecord, Producer};
use rdkafka::util::Timeout;
use serde::{Deserialize, Serialize};
use serde_json;
use tokio;
use tracing::{info, error, warn};

// Define our sensor data structure
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SensorReading {
    pub timestamp: u64,
    pub sensor_id: u32,
    pub temperature: f32,
    pub humidity: f32,
    pub pressure: f32,
}

#[derive(Debug)]
pub struct FileHeader {
    pub magic: [u8; 4],
    pub version: u32,
    pub record_count: u32,
}

#[derive(Debug)]
pub enum ParseError {
    IoError(std::io::Error),
    InvalidMagic(String),
    InvalidFormat(String),
    KafkaError(rdkafka::error::KafkaError),
    SerializationError(serde_json::Error),
}

impl From<std::io::Error> for ParseError {
    fn from(error: std::io::Error) -> Self {
        ParseError::IoError(error)
    }
}

impl From<rdkafka::error::KafkaError> for ParseError {
    fn from(error: rdkafka::error::KafkaError) -> Self {
        ParseError::KafkaError(error)
    }
}

impl From<serde_json::Error> for ParseError {
    fn from(error: serde_json::Error) -> Self {
        ParseError::SerializationError(error)
    }
}

// Helper function to read exact number of bytes
fn read_exact_bytes<R: Read>(reader: &mut R, count: usize) -> Result<Vec<u8>, ParseError> {
    let mut buffer = vec![0u8; count];
    reader.read_exact(&mut buffer)?;
    Ok(buffer)
}

// Helper functions to convert bytes to different types
fn bytes_to_u32(bytes: &[u8]) -> u32 {
    u32::from_le_bytes(bytes.try_into().unwrap())
}

fn bytes_to_u64(bytes: &[u8]) -> u64 {
    u64::from_le_bytes(bytes.try_into().unwrap())
}

fn bytes_to_f32(bytes: &[u8]) -> f32 {
    f32::from_le_bytes(bytes.try_into().unwrap())
}

impl FileHeader {
    fn parse<R: Read>(reader: &mut R) -> Result<Self, ParseError> {
        let magic_bytes = read_exact_bytes(reader, 4)?;
        let mut magic = [0u8; 4];
        magic.copy_from_slice(&magic_bytes);
        
        let version_bytes = read_exact_bytes(reader, 4)?;
        let version = bytes_to_u32(&version_bytes);
        
        let count_bytes = read_exact_bytes(reader, 4)?;
        let record_count = bytes_to_u32(&count_bytes);
        
        Ok(FileHeader {
            magic,
            version,
            record_count,
        })
    }
    
    fn magic_as_string(&self) -> String {
        String::from_utf8_lossy(&self.magic).to_string()
    }
}

impl SensorReading {
    fn parse<R: Read>(reader: &mut R) -> Result<Self, ParseError> {
        let timestamp_bytes = read_exact_bytes(reader, 8)?;
        let timestamp = bytes_to_u64(&timestamp_bytes);
        
        let sensor_id_bytes = read_exact_bytes(reader, 4)?;
        let sensor_id = bytes_to_u32(&sensor_id_bytes);
        
        let temperature_bytes = read_exact_bytes(reader, 4)?;
        let temperature = bytes_to_f32(&temperature_bytes);
        
        let humidity_bytes = read_exact_bytes(reader, 4)?;
        let humidity = bytes_to_f32(&humidity_bytes);
        
        let pressure_bytes = read_exact_bytes(reader, 4)?;
        let pressure = bytes_to_f32(&pressure_bytes);
        
        Ok(SensorReading {
            timestamp,
            sensor_id,
            temperature,
            humidity,
            pressure,
        })
    }
    
    pub fn size_in_bytes() -> usize {
        8 + 4 + 4 + 4 + 4 // timestamp + sensor_id + temp + humidity + pressure
    }

    pub fn to_json(&self) -> Result<String, ParseError> {
        Ok(serde_json::to_string(self)?)
    }

    pub fn get_kafka_key(&self) -> String {
        format!("sensor_{}", self.sensor_id)
    }
}

pub struct SensorParser;

impl SensorParser {
    pub fn parse_sensor_file(filename: &str) -> Result<Vec<SensorReading>, ParseError> {
        let file = File::open(filename)?;
        let mut reader = BufReader::new(file);
        
        let header = FileHeader::parse(&mut reader)?;
        
        if &header.magic != b"SENS" {
            return Err(ParseError::InvalidMagic(format!(
                "Expected 'SENS', found '{}'", 
                header.magic_as_string()
            )));
        }
        
        info!("Parsing sensor file: {} records, version {}", 
                header.record_count, header.version);
        
        let mut readings = Vec::new();
        
        for i in 0..header.record_count {
            match SensorReading::parse(&mut reader) {
                Ok(reading) => readings.push(reading),
                Err(e) => {
                    error!("Error parsing sensor record {}: {:?}", i, e);
                    break;
                }
            }
        }
        
        Ok(readings)
    }
}

pub struct KafkaProducerWrapper {
    producer: FutureProducer,
    topic: String,
}

impl KafkaProducerWrapper {
    pub fn new(brokers: &str, topic: &str) -> Result<Self, ParseError> {
        let producer: FutureProducer = ClientConfig::new()
            .set("bootstrap.servers", brokers)
            .set("message.timeout.ms", "5000")
            .set("acks", "all") // Wait for all replicas to acknowledge
            .set("retries", "3")
            .set("retry.backoff.ms", "100")
            .set("batch.size", "16384")
            .set("linger.ms", "10") // Small delay to batch messages
            .create()?;

        Ok(KafkaProducerWrapper {
            producer,
            topic: topic.to_string(),
        })
    }

    pub async fn send_sensor_reading(&self, reading: &SensorReading) -> Result<(), ParseError> {
        let json_payload = reading.to_json()?;
        let key = reading.get_kafka_key();

        let record = FutureRecord::to(&self.topic)
            .key(&key)
            .payload(&json_payload)
            .timestamp(reading.timestamp as i64);

        match self.producer.send(record, Timeout::After(Duration::from_secs(5))).await {
            Ok(delivery) => {
                info!("Message sent successfully: {:?}", delivery);
                Ok(())
            }
            Err((kafka_error, _)) => {
                error!("Failed to send message: {:?}", kafka_error);
                Err(ParseError::KafkaError(kafka_error))
            }
        }
    }

    pub async fn send_batch_sensor_readings(&self, readings: &[SensorReading]) -> Result<(), ParseError> {
        info!("Sending {} sensor readings to Kafka topic '{}'", readings.len(), self.topic);
        
        let mut successful_sends = 0;
        let mut failed_sends = 0;

        for (index, reading) in readings.iter().enumerate() {
            match self.send_sensor_reading(reading).await {
                Ok(_) => {
                    successful_sends += 1;
                    if (index + 1) % 100 == 0 {
                        info!("Sent {} of {} messages", index + 1, readings.len());
                    }
                }
                Err(e) => {
                    failed_sends += 1;
                    warn!("Failed to send reading for sensor {}: {:?}", reading.sensor_id, e);
                }
            }
        }

        info!("Batch send complete: {} successful, {} failed", successful_sends, failed_sends);

        // Flush the producer to ensure all messages are sent
        match self.producer.flush(Timeout::After(Duration::from_secs(10))) {
            Ok(_) => info!("Producer flushed successfully"),
            Err(e) => error!("Failed to flush producer: {:?}", e),
        }

        Ok(())
    }
}

#[tokio::main]
async fn main() -> Result<(), ParseError> {
    // Initialize tracing
    tracing_subscriber::fmt()
        .with_env_filter("info")
        .init();

    info!("Sensor Data Parser with Kafka Integration\n");
    
    // Configuration
    let data_path = std::env::var("DATA_PATH")
        .unwrap_or("../webway-data-generation/test_data/sensor_data.bin".to_string());
    let kafka_brokers = std::env::var("KAFKA_BROKERS")
        .unwrap_or("localhost:19092".to_string());
    let kafka_topic = std::env::var("KAFKA_TOPIC")
        .unwrap_or("sensor-data".to_string());

    info!("Configuration:");
    info!("  Data file: {}", data_path);
    info!("  Kafka brokers: {}", kafka_brokers);
    info!("  Kafka topic: {}", kafka_topic);

    // Parse sensor data
    match SensorParser::parse_sensor_file(&data_path) {
        Ok(readings) => {
            info!("Successfully parsed {} sensor readings", readings.len());
            
            if !readings.is_empty() {
                info!("Sample readings:");
                for (i, reading) in readings.iter().take(5).enumerate() {
                    info!("  {}: Sensor {} - {:.1}°C, {:.1}% humidity, {:.1} hPa", 
                            i + 1, reading.sensor_id, reading.temperature, 
                            reading.humidity, reading.pressure);
                }

                // Create Kafka producer and send data
                match KafkaProducerWrapper::new(&kafka_brokers, &kafka_topic) {
                    Ok(kafka_producer) => {
                        info!("Kafka producer created successfully");
                        
                        // Send all readings to Kafka
                        if let Err(e) = kafka_producer.send_batch_sensor_readings(&readings).await {
                            error!("Error sending data to Kafka: {:?}", e);
                        } else {
                            info!("All sensor data sent to Kafka successfully!");
                        }
                    }
                    Err(e) => {
                        error!("Failed to create Kafka producer: {:?}", e);
                    }
                }
            }
        }
        Err(e) => {
            error!("Error parsing sensor data: {:?}", e);
        }
    }
    
    Ok(())
}