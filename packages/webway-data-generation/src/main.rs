use std::fs::File;
use std::io::{Write, BufWriter};
use rand::prelude::*;

// Example data structures that might represent your domain
#[derive(Debug)]
struct SensorReading {
    timestamp: u64,
    sensor_id: u32,
    temperature: f32,
    humidity: f32,
    pressure: f32,
}

#[derive(Debug)]
struct LogEntry {
    timestamp: u64,
    level: u8, // 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR
    message_length: u16,
    message: String,
}

impl SensorReading {
    fn to_bytes(&self) -> Vec<u8> {
        let mut bytes = Vec::new();
        bytes.extend_from_slice(&self.timestamp.to_le_bytes());
        bytes.extend_from_slice(&self.sensor_id.to_le_bytes());
        bytes.extend_from_slice(&self.temperature.to_le_bytes());
        bytes.extend_from_slice(&self.humidity.to_le_bytes());
        bytes.extend_from_slice(&self.pressure.to_le_bytes());
        bytes
    }

    fn random() -> Self {
        let mut rng = thread_rng();
        SensorReading {
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_secs() + rng.gen_range(0..3600), // Random time within next hour
            sensor_id: rng.gen_range(1000..9999),
            temperature: rng.gen_range(15.0..35.0),
            humidity: rng.gen_range(30.0..80.0),
            pressure: rng.gen_range(990.0..1030.0),
        }
    }
}

impl LogEntry {
    fn to_bytes(&self) -> Vec<u8> {
        let mut bytes = Vec::new();
        bytes.extend_from_slice(&self.timestamp.to_le_bytes());
        bytes.push(self.level);
        bytes.extend_from_slice(&self.message_length.to_le_bytes());
        bytes.extend_from_slice(self.message.as_bytes());
        bytes
    }

    fn random() -> Self {
        let mut rng = thread_rng();
        let messages = vec![
            "System startup completed",
            "Database connection established",
            "Processing batch job",
            "Warning: High memory usage detected",
            "Error: Failed to connect to external service",
            "User authentication successful",
            "Cache cleared successfully",
            "Backup process initiated",
        ];
        
        let message = messages[rng.gen_range(0..messages.len())].to_string();
        LogEntry {
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_secs() + rng.gen_range(0..3600),
            level: rng.gen_range(0..4),
            message_length: message.len() as u16,
            message,
        }
    }
}

fn generate_sensor_data_file(filename: &str, count: usize) -> std::io::Result<()> {
    let file = File::create(filename)?;
    let mut writer = BufWriter::new(file);
    
    // Write a simple header: magic number + version + record count
    writer.write_all(b"SENS")?; // Magic number
    writer.write_all(&1u32.to_le_bytes())?; // Version
    writer.write_all(&(count as u32).to_le_bytes())?; // Record count
    
    for _ in 0..count {
        let reading = SensorReading::random();
        writer.write_all(&reading.to_bytes())?;
    }
    
    writer.flush()?;
    println!("Generated {} sensor readings in {}", count, filename);
    Ok(())
}

fn generate_log_data_file(filename: &str, count: usize) -> std::io::Result<()> {
    let file = File::create(filename)?;
    let mut writer = BufWriter::new(file);
    
    // Write header: magic number + version + record count
    writer.write_all(b"LOGS")?; // Magic number
    writer.write_all(&1u32.to_le_bytes())?; // Version
    writer.write_all(&(count as u32).to_le_bytes())?; // Record count
    
    for _ in 0..count {
        let log_entry = LogEntry::random();
        writer.write_all(&log_entry.to_bytes())?;
    }
    
    writer.flush()?;
    println!("Generated {} log entries in {}", count, filename);
    Ok(())
}

fn generate_mixed_data_file(filename: &str, count: usize) -> std::io::Result<()> {
    let file = File::create(filename)?;
    let mut writer = BufWriter::new(file);
    let mut rng = thread_rng();
    
    // Write header: magic number + version + record count
    writer.write_all(b"MIXD")?; // Magic number
    writer.write_all(&1u32.to_le_bytes())?; // Version
    writer.write_all(&(count as u32).to_le_bytes())?; // Record count
    
    for _ in 0..count {
        if rng.gen_bool(0.6) {
            // 60% chance of sensor data
            writer.write_all(&0u8.to_le_bytes())?; // Type indicator
            let reading = SensorReading::random();
            writer.write_all(&reading.to_bytes())?;
        } else {
            // 40% chance of log data
            writer.write_all(&1u8.to_le_bytes())?; // Type indicator
            let log_entry = LogEntry::random();
            writer.write_all(&log_entry.to_bytes())?;
        }
    }
    
    writer.flush()?;
    println!("Generated {} mixed records in {}", count, filename);
    Ok(())
}

fn generate_raw_bytes_file(filename: &str, size_kb: usize) -> std::io::Result<()> {
    let file = File::create(filename)?;
    let mut writer = BufWriter::new(file);
    let mut rng = thread_rng();
    
    let bytes_to_write = size_kb * 1024;
    let mut buffer = vec![0u8; 1024]; // 1KB buffer
    
    for _ in 0..(bytes_to_write / 1024) {
        rng.fill_bytes(&mut buffer);
        writer.write_all(&buffer)?;
    }
    
    // Handle remaining bytes
    let remaining = bytes_to_write % 1024;
    if remaining > 0 {
        buffer.resize(remaining, 0);
        rng.fill_bytes(&mut buffer);
        writer.write_all(&buffer)?;
    }
    
    writer.flush()?;
    println!("Generated {}KB of random data in {}", size_kb, filename);
    Ok(())
}

fn main() -> std::io::Result<()> {
    // Create output directory if it doesn't exist
    std::fs::create_dir_all("test_data")?;
    
    // Generate different types of test data
    generate_sensor_data_file("test_data/sensor_data.bin", 1000)?;
    generate_log_data_file("test_data/log_data.bin", 500)?;
    generate_mixed_data_file("test_data/mixed_data.bin", 750)?;
    generate_raw_bytes_file("test_data/random_data.bin", 100)?; // 100KB
    
    println!("\nAll test data files generated successfully!");
    println!("Files created in ./test_data/ directory:");
    println!("- sensor_data.bin: Structured sensor readings");
    println!("- log_data.bin: Variable-length log entries");
    println!("- mixed_data.bin: Mixed sensor and log data");
    println!("- random_data.bin: Pure random binary data");
    
    Ok(())
}