# Testing and Linting Guide

This guide covers all the testing, linting, and code quality tools available for the `webway-parser` project.

## 🧪 Testing Overview

The project includes multiple testing strategies to ensure code quality and reliability:

- **Unit Tests** - Fast tests for individual functions and components
- **Integration Tests** - Tests with real Kafka/database connections
- **Property-Based Tests** - Randomized testing for edge cases
- **Performance Tests** - Benchmarking and performance validation

## 📋 Quick Commands

```bash
# Basic testing
cargo test                              # Run all unit tests
cargo test --lib                        # Run only library tests
cargo test --features property-tests    # Include property-based tests
cargo test --features integration-tests # Include integration tests (needs Docker)
cargo test --features full-test-suite   # Run everything

# Linting and formatting
cargo fmt                               # Format code
cargo fmt -- --check                   # Check formatting without changes
cargo clippy                           # Run linter
cargo clippy -- -D warnings            # Treat warnings as errors

# Documentation
cargo doc                               # Generate documentation
cargo doc --open                       # Generate and open docs
cargo test --doc                       # Test code examples in docs

# Coverage (requires cargo-llvm-cov)
cargo llvm-cov --all-features              # Quick coverage report in terminal
cargo llvm-cov --all-features --html       # Generate HTML coverage report
cargo llvm-cov --all-features --lcov --output-path lcov.info  # LCOV format for CI/CD

# Docker coverage
docker run --rm your-image cargo llvm-cov --all-features  # Run coverage in container
```

## 🔧 Setup Requirements

### Core Tools

Install these tools for the full development experience:

```bash
# Essential tools
rustup component add rustfmt clippy

# Coverage tool (optional)
cargo install cargo-llvm-cov

# Security audit (optional)  
cargo install cargo-audit

# Dependency checking (optional)
cargo install cargo-outdated
```

### Docker for Integration Tests

Integration tests require Docker to spin up test containers:

```bash
# Start Docker daemon
docker --version  # Verify Docker is installed

# Pull test images (optional - will happen automatically)
docker pull redpandadata/redpanda:latest
docker pull postgres:15
```

## 📊 Test Categories

### Unit Tests (`cargo test`)

Fast tests that don't require external dependencies:

```rust
#[test]
fn test_automation_data_creation() {
    let data = AutomationData::new_deterministic(123, 456, 1234567890);
    assert_eq!(data.message_key, 123);
    assert_eq!(data.sequence_number, 456);
}
```

**Location**: Inline with source code using `#[cfg(test)]`

### Integration Tests (`cargo test --features integration-tests`)

Tests that require real external services:

```rust
#[tokio::test]
#[ignore] // Ignored by default, run with --ignored flag
async fn test_kafka_integration() {
    // Spins up real Kafka container
    let kafka = testcontainers::start_kafka().await;
    // ... test with real Kafka
}
```

**Requirements**: 
- Docker running
- `--features integration-tests` flag
- Use `--ignored` to run tests marked with `#[ignore]`

### Property-Based Tests (`cargo test --features property-tests`)

Randomized testing to find edge cases:

```rust
#[quickcheck]
fn prop_serialization_roundtrip(key: i32, seq: i32) -> bool {
    let data = AutomationData::new_deterministic(key, seq, 12345);
    let encoded = encode(&data);
    let decoded = decode(&encoded);
    data == decoded
}
```

**Location**: `tests/property_tests.rs`

## 🧹 Code Quality Tools

### Formatting with `rustfmt`

```bash
# Format all code
cargo fmt

# Check if code is formatted (CI usage)
cargo fmt -- --check

# Format with custom config
cargo fmt -- --config hard_tabs=true
```

**Configuration**: Create `.rustfmt.toml` for custom formatting rules:

```toml
max_width = 100
hard_tabs = false
tab_spaces = 4
newline_style = "Unix"
use_small_heuristics = "Default"
```

### Linting with `clippy`

```bash
# Basic linting
cargo clippy

# Strict linting (treat warnings as errors)
cargo clippy -- -D warnings

# Fix automatically fixable issues
cargo clippy --fix

# Lint tests too
cargo clippy --tests
```

**Common clippy configurations** in `Cargo.toml`:

```toml
[lints.clippy]
# Deny these lints
unwrap_used = "deny"
expect_used = "deny"
panic = "deny"

# Allow these lints
too_many_arguments = "allow"
module_name_repetitions = "allow"
```

### Documentation

```bash
# Generate docs
cargo doc

# Generate and open docs
cargo doc --open

# Test documentation examples
cargo test --doc

# Generate docs with private items
cargo doc --document-private-items
```

## 🎯 Testing Best Practices

### Test Organization

```
src/
├── lib.rs
├── kafka_producer.rs  # Contains #[cfg(test)] mod tests
└── automation_data.rs # Contains #[cfg(test)] mod tests

tests/
├── integration_tests.rs      # Integration tests
├── property_tests.rs         # Property-based tests
└── common/
    └── mod.rs                # Shared test utilities
```

### Test Naming Convention

```rust
#[test]
fn test_function_name_expected_behavior() {
    // Given
    let input = setup_test_data();
    
    // When
    let result = function_under_test(input);
    
    // Then
    assert_eq!(result, expected_value);
}
```

### Async Testing

```rust
#[tokio::test]
async fn test_async_function() {
    let result = async_function().await;
    assert!(result.is_ok());
}

#[tokio::test]
#[serial] // Run sequentially if tests conflict
async fn test_with_shared_resource() {
    // Test that can't run in parallel
}
```

### Mocking

```rust
use mockall::automock;

#[automock]
trait KafkaClient {
    async fn send(&self, data: &[u8]) -> Result<(), Error>;
}

#[tokio::test]
async fn test_with_mock() {
    let mut mock = MockKafkaClient::new();
    mock.expect_send()
        .times(1)
        .returning(|_| Ok(()));
    
    // Test using mock
}
```

## 🛠 IDE Configuration

### VS Code Settings

Create `.vscode/settings.json`:

```json
{
    "rust-analyzer.check.command": "clippy",
    "rust-analyzer.cargo.features": "all",
    "rust-analyzer.procMacro.enable": true,
    "files.insertFinalNewline": true,
    "editor.formatOnSave": true
}
```

### VS Code Tasks

Create `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "cargo test",
            "type": "shell",
            "command": "cargo",
            "args": ["test"],
            "group": "test"
        },
        {
            "label": "cargo clippy",
            "type": "shell", 
            "command": "cargo",
            "args": ["clippy"]
        }
    ]
}
```

## 🐛 Debugging Tests

### Test Output

```bash
# Show test output
cargo test -- --nocapture

# Show test output for specific test
cargo test test_name -- --nocapture

# Run tests with backtrace on panic
RUST_BACKTRACE=1 cargo test

# Run single test
cargo test test_specific_function

# Run tests matching pattern
cargo test kafka
```

### Debug Configuration

```rust
#[test]
fn debug_test() {
    env_logger::init(); // Initialize logging in tests
    
    let data = create_test_data();
    println!("Debug: data = {:?}", data); // Will show with --nocapture
    
    assert_eq!(data.field, expected_value);
}
```

## 📚 Additional Resources

- [Rust Testing Guide](https://doc.rust-lang.org/book/ch11-00-testing.html)
- [Clippy Lint List](https://rust-lang.github.io/rust-clippy/master/)
- [Property-Based Testing with QuickCheck](https://docs.rs/quickcheck/latest/quickcheck/)
- [Testcontainers for Rust](https://docs.rs/testcontainers/latest/testcontainers/)
- [Criterion Benchmarking](https://bheisler.github.io/criterion.rs/book/)

## 🔄 Development Workflow

1. **Write tests first** (TDD approach)
2. **Run `cargo test`** frequently during development
3. **Use `cargo clippy`** to catch common issues
4. **Format with `cargo fmt`** before committing
5. **Run integration tests** before pushing
6. **Check coverage** periodically to identify untested code

Happy testing! 🎉