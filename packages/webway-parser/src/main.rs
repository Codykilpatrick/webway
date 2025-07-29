// No build.rs needed! No .proto file needed! Just use prost derive macros directly.

use prost::Message;
use rdkafka::producer::{FutureProducer, FutureRecord, Producer};
use rdkafka::admin::{AdminClient, AdminOptions, NewTopic, TopicReplication};
use rdkafka::ClientConfig;
use std::time::{SystemTime, UNIX_EPOCH, Duration};
use rand::Rng;
use tokio;

// Define your protobuf struct directly using prost derive macros
#[derive(Clone, PartialEq, Message)]
pub struct AutomationData {
    #[prost(int32, tag = "1")]
    pub message_key: i32,
    
    #[prost(int32, tag = "2")]
    pub sequence_number: i32,
    
    #[prost(uint64, tag = "3")]
    pub sys_timestamp: u64,
    
    // packed = true gives you the same optimization as packed repeated fields
    #[prost(float, repeated, packed = "true", tag = "4")]
    pub normalized_data: Vec<f32>,
    
    #[prost(float, repeated, packed = "true", tag = "5")]
    pub unnormalized_data: Vec<f32>,
}

impl AutomationData {
    fn new(message_key: i32, sequence_number: i32) -> Self {
        let mut rng = rand::thread_rng();
        
        // Generate 780,000 random floats for normalized data (between 0.0 and 1.0)
        let normalized_data: Vec<f32> = (0..780_000)
            .map(|_| rng.gen::<f32>())
            .collect();
        
        // Generate 780,000 random floats for unnormalized data
        let unnormalized_data: Vec<f32> = (0..780_000)
            .map(|_| rng.gen_range(-1000.0..1000.0))
            .collect();
        
        // Get current Unix timestamp
        let sys_timestamp = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("Time went backwards")
            .as_secs();
        
        AutomationData {
            message_key,
            sequence_number,
            sys_timestamp,
            normalized_data,
            unnormalized_data,
        }
    }
}

async fn create_topic_if_not_exists(bootstrap_servers: &str, topic_name: &str) -> Result<(), Box<dyn std::error::Error>> {
    let admin_client: AdminClient<_> = ClientConfig::new()
        .set("bootstrap.servers", bootstrap_servers)
        .create()?;
    
    // Create topic with large message support
    let topic_config = vec![
        ("max.message.bytes", "10485760"), // 10MB
        ("compression.type", "lz4"),
    ];
    
    let new_topic = NewTopic {
        name: topic_name,
        num_partitions: 3, // Multiple partitions for better parallelism
        replication: TopicReplication::Fixed(1),
        config: topic_config,
    };
    
    let admin_opts = AdminOptions::new().request_timeout(Some(Duration::from_secs(10)));
    
    match admin_client.create_topics(&[new_topic], &admin_opts).await {
        Ok(results) => {
            for result in results {
                match result {
                    Ok(topic) => println!("✅ Topic '{}' created successfully", topic),
                    Err((topic, err)) => {
                        // Topic might already exist, which is fine
                        if err.to_string().contains("already exists") {
                            println!("📋 Topic '{}' already exists", topic);
                        } else {
                            println!("⚠️  Warning creating topic '{}': {}", topic, err);
                        }
                    }
                }
            }
        }
        Err(err) => {
            println!("⚠️  Warning during topic creation: {}", err);
        }
    }
    
    Ok(())
}

async fn create_kafka_producer() -> Result<FutureProducer, rdkafka::error::KafkaError> {
    ClientConfig::new()
        .set("bootstrap.servers", "localhost:19092")
        .set("message.max.bytes", "10485760") // 10MB max message size
        .set("compression.type", "lz4") // Enable LZ4 compression
        .set("batch.size", "1048576") // 1MB batch size
        .set("linger.ms", "10") // Small delay to allow batching
        .create()
}

async fn send_to_redpanda(producer: &FutureProducer, data: &AutomationData, topic: &str) -> Result<(), Box<dyn std::error::Error>> {
    // Serialize to protobuf bytes
    let mut buf = Vec::new();
    data.encode(&mut buf)?;
    
    // Print detailed message size info
    let raw_size = buf.len();
    let size_mb = raw_size as f64 / 1024.0 / 1024.0;
    
    println!("📦 Message {} - Raw protobuf size: {} bytes ({:.2} MB)", 
             data.sequence_number, raw_size, size_mb);
    
    // Create longer-lived key string
    let key = data.sequence_number.to_string();
    
    // Create the record
    let record = FutureRecord::to(topic)
        .key(&key)
        .payload(&buf);
    
    // Send the message
    match producer.send(record, tokio::time::Duration::from_secs(5)).await {
        Ok(delivery) => {
            println!("✅ Message {} sent successfully!", data.sequence_number);
            println!("   📍 Partition: {}, Offset: {}", delivery.0, delivery.1);
            Ok(())
        }
        Err((err, _)) => {
            eprintln!("❌ Failed to send message {}: {}", data.sequence_number, err);
            Err(Box::new(err))
        }
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize tracing
    tracing_subscriber::fmt::init();
    
    const MESSAGE_KEY: i32 = 12345;
    const TOPIC: &str = "automation-data";
    const BOOTSTRAP_SERVERS: &str = "localhost:19092";
    
    // Create topic with proper large message configuration
    create_topic_if_not_exists(BOOTSTRAP_SERVERS, TOPIC).await?;
    
    // Create Kafka producer
    let producer = create_kafka_producer().await?;
    
    // Generate and send 10 AutomationData messages
    let mut total_raw_size = 0;
    let start_time = std::time::Instant::now();
    
    for i in 0..10 {
        let data = AutomationData::new(MESSAGE_KEY, i);
        
        // Calculate raw protobuf size
        let mut buf = Vec::new();
        data.encode(&mut buf)?;
        let raw_size = buf.len();
        total_raw_size += raw_size;
        
        // Calculate raw data size (before any encoding)
        let raw_data_size = (data.normalized_data.len() * 4) + (data.unnormalized_data.len() * 4) + 4 + 4 + 8; // f32s + i32s + u64
        let raw_data_size_mb = raw_data_size as f64 / 1024.0 / 1024.0;
        
        // Show detailed message info
        println!("\n🔄 Generated AutomationData #{}", i);
        println!("   📊 Message Key: {}", data.message_key);
        println!("   🔢 Sequence Number: {}", data.sequence_number);
        println!("   ⏰ Timestamp: {}", data.sys_timestamp);
        println!("   📈 Normalized Data: {} floats", data.normalized_data.len());
        println!("   📉 Unnormalized Data: {} floats", data.unnormalized_data.len());
        println!("   📁 Raw Data Size: {} bytes ({:.2} MB)", raw_data_size, raw_data_size_mb);
        println!("   📦 Raw Protobuf Size: {} bytes ({:.2} MB)", raw_size, raw_size as f64 / 1024.0 / 1024.0);
        
        // Send to Redpanda (this will print additional size info)
        send_to_redpanda(&producer, &data, TOPIC).await?;
        
        // Small delay between messages
        tokio::time::sleep(tokio::time::Duration::from_millis(100)).await;
    }
    
    // Print summary statistics
    let elapsed = start_time.elapsed();
    let avg_size = total_raw_size / 10;
    let total_size_mb = total_raw_size as f64 / 1024.0 / 1024.0;
    let avg_size_mb = avg_size as f64 / 1024.0 / 1024.0;
    
    println!("\n📊 SUMMARY STATISTICS:");
    println!("   📨 Total messages sent: 10");
    println!("   📦 Total raw data size: {} bytes ({:.2} MB)", total_raw_size, total_size_mb);
    println!("   📊 Average message size: {} bytes ({:.2} MB)", avg_size, avg_size_mb);
    println!("   ⏱️  Total time: {:.2}s", elapsed.as_secs_f64());
    println!("   🚀 Throughput: {:.2} MB/s", total_size_mb / elapsed.as_secs_f64());
    
    // Flush any remaining messages
    let _ = producer.flush(tokio::time::Duration::from_secs(5));
    
    println!("\n🎉 All messages sent successfully!");
    println!("💡 Note: Actual network transfer size will be smaller due to LZ4 compression");
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::{SystemTime, UNIX_EPOCH};

    #[test]
    fn test_automation_data_creation() {
        let data = AutomationData::new(12345, 1);
        
        assert_eq!(data.message_key, 12345);
        assert_eq!(data.sequence_number, 1);
        assert_eq!(data.normalized_data.len(), 780_000);
        assert_eq!(data.unnormalized_data.len(), 780_000);
        
        // Check that timestamp is reasonable (within last minute)
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();
        assert!(data.sys_timestamp <= now);
        assert!(data.sys_timestamp > now - 60); // Created within last minute
    }

    #[test]
    fn test_automation_data_normalized_range() {
        let data = AutomationData::new(100, 2);
        
        // Check that normalized data is in expected range [0.0, 1.0)
        for &value in &data.normalized_data {
            assert!(value >= 0.0 && value < 1.0, "Normalized value {} out of range", value);
        }
    }

    #[test]
    fn test_automation_data_unnormalized_range() {
        let data = AutomationData::new(200, 3);
        
        // Check that unnormalized data is in expected range [-1000.0, 1000.0)
        for &value in &data.unnormalized_data {
            assert!(value >= -1000.0 && value < 1000.0, "Unnormalized value {} out of range", value);
        }
    }

    #[test]
    fn test_automation_data_serialization() {
        let data = AutomationData::new(300, 4);
        
        // Test protobuf serialization
        let mut buf = Vec::new();
        data.encode(&mut buf).expect("Should encode successfully");
        
        // Should create a non-empty buffer
        assert!(!buf.is_empty());
        
        // Should be able to decode back
        let decoded = AutomationData::decode(&buf[..]).expect("Should decode successfully");
        assert_eq!(decoded.message_key, data.message_key);
        assert_eq!(decoded.sequence_number, data.sequence_number);
        assert_eq!(decoded.sys_timestamp, data.sys_timestamp);
        assert_eq!(decoded.normalized_data.len(), data.normalized_data.len());
        assert_eq!(decoded.unnormalized_data.len(), data.unnormalized_data.len());
    }

    #[test]
    fn test_automation_data_different_sequence_numbers() {
        let data1 = AutomationData::new(400, 5);
        let data2 = AutomationData::new(400, 6);
        
        // Should have same message key but different sequence numbers
        assert_eq!(data1.message_key, data2.message_key);
        assert_ne!(data1.sequence_number, data2.sequence_number);
        
        // Data should be different (very unlikely to be identical with random generation)
        assert_ne!(data1.normalized_data, data2.normalized_data);
        assert_ne!(data1.unnormalized_data, data2.unnormalized_data);
    }

    #[tokio::test]
    async fn test_create_kafka_producer() {
        // This test will only work if Kafka/Redpanda is running
        // We'll just test that the configuration doesn't panic
        let result = create_kafka_producer().await;
        
        // We expect this might fail if Kafka isn't running, but it shouldn't panic
        // The function should return either Ok or a proper error
        match result {
            Ok(_) => println!("Kafka producer created successfully"),
            Err(e) => println!("Expected error when Kafka not available: {}", e),
        }
    }
}