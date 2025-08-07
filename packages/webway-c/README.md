# Webway C/C++ Data Parser

High-performance data parser and Kafka producer implemented in C/C++. This package provides a development environment and container setup for C/C++ programs similar to the Rust `webway-parser` package.

## Features

- **High-Performance Data Generation**: Creates large datasets with 780,000 normalized and unnormalized float values
- **Kafka Integration**: Produces messages to Kafka/Redpanda with LZ4 compression
- **Container-Based Development**: Docker environment with all necessary C/C++ tools
- **Comprehensive Testing**: Built-in test suite with memory checking via Valgrind
- **Modern Build System**: Makefile with development, testing, and debugging targets

## Quick Start

### Using Docker (Recommended)

```bash
# Build and run the container
docker-compose up --build

# Run the program
docker exec -it webway-c make run

# Run tests
docker exec -it webway-c make test

# Run with memory checking
docker exec -it webway-c make valgrind
```

### Local Development

```bash
# Install dependencies (Ubuntu/Debian)
make install-deps

# Build the project
make build

# Run the program
make run

# Run tests
make test
```

## Project Structure

```
webway-c/
├── src/
│   ├── main.c              # Main application entry point
│   ├── automation_data.h   # AutomationData structure definition
│   ├── automation_data.c   # AutomationData implementation
│   ├── kafka_producer.h    # Kafka producer interface
│   └── kafka_producer.c    # Kafka producer implementation
├── test_data/              # Test data files (shared with other packages)
├── output/                 # Output directory for results
├── Dockerfile              # Container definition
├── docker-compose.yaml     # Container orchestration
├── Makefile               # Build system
├── package.json           # Node.js package management
└── README.md              # This file
```

## AutomationData Structure

The core data structure contains:

- `message_key`: 32-bit integer identifier
- `sequence_number`: 32-bit sequence number
- `sys_timestamp`: 64-bit Unix timestamp
- `normalized_data`: 780,000 float values (0.0 to 1.0)
- `unnormalized_data`: 780,000 float values (-1000.0 to 1000.0)

## Build Targets

- `make build` - Build the project
- `make debug` - Build with debug symbols
- `make release` - Build with optimizations
- `make run` - Build and run the program
- `make test` - Run test suite
- `make valgrind` - Run with memory checking
- `make gdb-debug` - Start GDB debugger
- `make format` - Format source code
- `make lint` - Run static analysis
- `make clean` - Clean build artifacts

## Dependencies

### Runtime Dependencies
- librdkafka (Kafka client)
- libprotobuf-c (Protocol Buffers)
- libjson-c (JSON parsing)
- libcurl (HTTP client)
- liblz4 (LZ4 compression)
- libssl/libcrypto (SSL/TLS)

### Development Dependencies
- gcc/g++ (C/C++ compilers)
- clang (Alternative compiler)
- make/cmake (Build systems)
- gdb (Debugger)
- valgrind (Memory checker)
- clang-format (Code formatter)
- cppcheck (Static analyzer)

## Usage Examples

### Basic Usage

```bash
# Run the default Kafka producer demo
./build/webway-parser

# Run tests
./build/webway-parser --test

# Show help
./build/webway-parser --help
```

### Docker Usage

```bash
# Start the container
docker-compose up -d

# Execute commands in the container
docker exec -it webway-c make run
docker exec -it webway-c make test
docker exec -it webway-c make valgrind

# Development with live code reloading
docker exec -it webway-c bash
```

## Configuration

Environment variables:

- `LOG_LEVEL`: Logging level (default: debug)
- `DATA_PATH`: Path to test data files
- `KAFKA_BROKERS`: Kafka bootstrap servers (default: redpanda-0:9092)
- `KAFKA_TOPIC`: Kafka topic name (default: sensor-data)
- `CC`: C compiler (default: gcc)
- `CXX`: C++ compiler (default: g++)

## Performance

The program generates and sends large messages (~6.3 MB each) with:

- **Throughput**: Optimized for high-volume data streaming
- **Compression**: LZ4 compression reduces network overhead
- **Memory Management**: Careful allocation/deallocation to prevent leaks
- **Batch Processing**: Configurable batching for improved performance

## Development

### Code Style

The project follows these conventions:
- K&R C style with 4-space indentation
- Consistent naming (snake_case for functions, PascalCase for types)
- Comprehensive error handling
- Memory safety practices

### Adding Features

1. Add new functions to appropriate header files
2. Implement in corresponding `.c` files
3. Update the Makefile if needed
4. Add tests for new functionality
5. Update documentation

### Testing

The test suite includes:
- Data structure validation
- Serialization/deserialization tests
- Range checking for generated data
- Memory leak detection with Valgrind

## Troubleshooting

### Common Issues

1. **Kafka Connection Failed**
   ```bash
   # Ensure Redpanda is running
   docker-compose -f ../webway-redpanda/docker-compose.yaml up -d
   ```

2. **Build Errors**
   ```bash
   # Install missing dependencies
   make install-deps
   ```

3. **Memory Leaks**
   ```bash
   # Run with Valgrind
   make valgrind
   ```

## Integration with Webway Ecosystem

This package integrates with:
- **webway-parser**: Rust implementation for comparison
- **webway-data-generation**: Shared test data
- **webway-redpanda**: Kafka/Redpanda infrastructure

## License

MIT License - see LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests and ensure no memory leaks
5. Submit a pull request

## Version History

- **1.0.0**: Initial C/C++ implementation
  - AutomationData structure and serialization
  - Kafka producer integration
  - Docker containerization
  - Comprehensive test suite