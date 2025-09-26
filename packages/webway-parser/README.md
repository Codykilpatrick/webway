## Directory strucutre

```bash
src/
├── lib.rs                     // Library code
├── bin/
│   ├── apb19_file_parser.rs   // Binary for APB19 parsing
│   └── apb21_file_parser.rs   // Binary for APB21 parsing  
├── protocols/
│   ├── mod.rs
│   ├── common/
│   │   ├── mod.rs
│   │   ├── primitives.rs
│   │   ├── errors.rs
│   │   └── types.rs
│   ├── ti18-apb19/
│   │   ├── mod.rs
│   │   ├── types.rs
│   │   └── parsers.rs
│   └── ti20-apb21/
│       ├── mod.rs
│       ├── types.rs
│       └── parsers.rs
└── converters/
    ├── mod.rs
    ├── arrow.rs
    └── protobuf.rs
```

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

# Coverage tool 
cargo install cargo-llvm-cov
```

## 🧹 Code Quality Tools

### Formatting with `rustfmt`

```bash
# Format all code
cargo fmt

# Check if code is formatted (CI usage)
cargo fmt -- --check
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

## 📚 Additional Resources

- [Rust Testing Guide](https://doc.rust-lang.org/book/ch11-00-testing.html)
- [Clippy Lint List](https://rust-lang.github.io/rust-clippy/master/)
- [Property-Based Testing with QuickCheck](https://docs.rs/quickcheck/latest/quickcheck/)
- [Testcontainers for Rust](https://docs.rs/testcontainers/latest/testcontainers/)
- [Criterion Benchmarking](https://bheisler.github.io/criterion.rs/book/)
