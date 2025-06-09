use std::fs::File;
use std::io::{Read, BufReader};
use std::convert::TryInto;

// Define our sensor data structure
#[derive(Debug, Clone)]
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
}

impl From<std::io::Error> for ParseError {
    fn from(error: std::io::Error) -> Self {
        ParseError::IoError(error)
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
        
        println!("Parsing sensor file: {} records, version {}", 
                header.record_count, header.version);
        
        let mut readings = Vec::new();
        
        for i in 0..header.record_count {
            match SensorReading::parse(&mut reader) {
                Ok(reading) => readings.push(reading),
                Err(e) => {
                    eprintln!("Error parsing sensor record {}: {:?}", i, e);
                    break;
                }
            }
        }
        
        Ok(readings)
    }
}

fn main() -> Result<(), ParseError> {
    println!("Sensor Data Parser\n");
    
    // Parse sensor data
    match SensorParser::parse_sensor_file("test_data/sensor_data.bin") {
        Ok(readings) => {
            println!("Successfully parsed {} sensor readings", readings.len());
            if !readings.is_empty() {
                println!("Sample readings:");
                for (i, reading) in readings.iter().enumerate() {
                    // println!("{:?}", reading);
                    println!("  {}: Sensor {} - {:.1}°C, {:.1}% humidity, {:.1} hPa", 
                            i + 1, reading.sensor_id, reading.temperature, 
                            reading.humidity, reading.pressure);
                }

                
            }
        }
        Err(e) => eprintln!("Error parsing sensor data: {:?}", e),
    }
    
    Ok(())
}