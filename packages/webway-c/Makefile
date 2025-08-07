# Webway C/C++ Data Parser - Makefile

# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -g
LDFLAGS = -lrdkafka -ljson-c -lcurl -lz -llz4 -lssl -lcrypto -pthread -lffi -ldl -lzmq -luv -lev -levent -lwebsockets -lyaml -lxml2 -lprotobuf-c

# Debug and Release configurations
DEBUG_FLAGS = -DDEBUG -g -O0
RELEASE_FLAGS = -DNDEBUG -O3 -march=native

.PHONY: all build debug release run test valgrind gdb-debug clean format lint help

# Default target
all: build

# Main build target
build: webway-parser

webway-parser: build/automation_data.o build/kafka_producer.o build/main.o
	@mkdir -p build
	$(CC) build/automation_data.o build/kafka_producer.o build/main.o -o build/webway-parser $(LDFLAGS)
	@echo "✅ Build completed successfully!"

# Object files
build/automation_data.o: src/automation_data.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/automation_data.c -o build/automation_data.o

build/kafka_producer.o: src/kafka_producer.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/kafka_producer.c -o build/kafka_producer.o

build/main.o: src/main.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: webway-parser

# Release build
release: CFLAGS += $(RELEASE_FLAGS)
release: CXXFLAGS += $(RELEASE_FLAGS)
release: webway-parser

# Run the program
run: webway-parser
	./build/webway-parser

# Testing
test: webway-parser
	@echo "Running basic functionality tests..."
	./build/webway-parser --test
	@echo "All tests passed!"

# Memory checking with Valgrind
valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/webway-parser

# Debug with GDB
gdb-debug: debug
	gdb ./build/webway-parser

# Code formatting
format:
	@echo "Formatting C/C++ code..."
	find src -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -i

format-check:
	@echo "Checking C/C++ code formatting..."
	find src -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format --dry-run --Werror

# Linting
lint:
	@echo "Running static analysis..."
	find src -name "*.c" -o -name "*.cpp" | xargs cppcheck --enable=all --error-exitcode=1

# Clean build artifacts
clean:
	rm -rf build
	rm -rf output/*
	@echo "Build artifacts cleaned"

# Install dependencies (for development setup)
install-deps:
	@echo "Installing C/C++ development dependencies..."
	sudo apt-get update
	sudo apt-get install -y build-essential gcc g++ clang make cmake gdb valgrind
	sudo apt-get install -y librdkafka-dev libprotobuf-dev protobuf-compiler
	sudo apt-get install -y libjson-c-dev libcurl4-openssl-dev zlib1g-dev liblz4-dev
	sudo apt-get install -y clang-format cppcheck

# Help
help:
	@echo "Available targets:"
	@echo "  build        - Build the project (default)"
	@echo "  debug        - Build with debug flags"
	@echo "  release      - Build with release optimizations"
	@echo "  run          - Build and run the program"
	@echo "  test         - Run tests"
	@echo "  valgrind     - Run with Valgrind memory checking"
	@echo "  gdb-debug    - Start GDB debugger"
	@echo "  format       - Format source code"
	@echo "  format-check - Check code formatting"
	@echo "  lint         - Run static analysis"
	@echo "  clean        - Clean build artifacts"
	@echo "  install-deps - Install development dependencies"
	@echo "  help         - Show this help message"