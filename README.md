# WebWay Monorepo

A high-performance monorepo containing data processing utilities built with Rust, including Redpanda cluster integration, binary data parsing, and data generation tools.

## 🚀 Overview

WebWay is a collection of high-performance data processing utilities designed for efficient data manipulation and generation. The project leverages Rust's performance capabilities while providing a modern JavaScript/TypeScript interface.

### Key Features

- **High-Performance Data Parser**: Rust-based binary data parsing with exceptional speed
- **Data Generation Utilities**: Efficient tools for generating test and mock data
- **Redpanda Integration**: Seamless integration with Redpanda clusters
- **TypeScript Support**: Full TypeScript definitions and support
- **Monorepo Architecture**: Organized workspace structure with Yarn workspaces

## 📦 Packages

This monorepo contains the following packages:

### [@webway/data-parser](./packages/webway-parser)
High-performance data parser built with Rust for processing binary data formats.

**Features:**
- Lightning-fast binary data parsing
- Memory-efficient processing
- TypeScript bindings
- Comprehensive error handling

### [@webway/data-generation](./packages/webway-data-generation)
High-performance data generation utilities for creating test data and mock datasets.

**Features:**
- Fast data generation algorithms
- Customizable data patterns
- Multiple output formats
- Rust-powered performance

## 🛠️ Prerequisites

Before getting started, ensure you have the following installed:

- **Node.js** >= 18.0.0
- **Yarn** >= 1.22.0 (recommended: 3.6.4)
- **Rust** (latest stable version)
- **Cargo** (comes with Rust)

## 📋 Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/your-username/webway.git
   cd webway
   ```

2. **Install dependencies:**
   ```bash
   yarn install
   ```

3. **Build all packages:**
   ```bash
   yarn build
   ```

## 🔧 Development

### Available Scripts

- `yarn lint` - Lint code using Cargo clippy
- `yarn prettier:check` - Check code formatting
- `yarn prettier:fix` - Fix code formatting

### Working with Individual Packages

Navigate to specific packages to work on them individually:

```bash
# Work on the data parser
cd packages/webway-parser
cargo build --release
cargo test

# Work on data generation
cd packages/webway-data-generation
cargo build --release
cargo test
```

### Package-Specific Commands

Each package supports the following Cargo commands:

- `cargo build --release` - Build optimized release version
- `cargo run` - Run the package in development mode
- `cargo test` - Run package tests
- `cargo clippy -- -D warnings` - Lint with Clippy
- `cargo fmt` - Format Rust code
- `cargo clean` - Clean build artifacts

## 🏗️ Project Structure

```
webway/
├── packages/
│   ├── webway-parser/          # High-performance data parser
│   │   ├── src/                # Rust source code
│   │   ├── test_data/          # Test data files
│   │   ├── Cargo.toml          # Rust package configuration
│   │   └── package.json        # Node.js package configuration
│   └── webway-data-generation/ # Data generation utilities
│       ├── src/                # Rust source code
│       ├── test_data/          # Test data files
│       ├── Cargo.toml          # Rust package configuration
│       └── package.json        # Node.js package configuration
├── package.json                # Root package configuration
├── yarn.lock                   # Yarn lockfile
└── README.md                   # This file
```

## 🚀 Usage


## 🧪 Testing

Run tests for all packages:

```bash
yarn test
```

Run tests for individual packages:

```bash
# Test data parser
yarn workspace @webway/data-parser run test

# Test data generation
yarn workspace @webway/data-generation run test
```

## 🔍 Code Quality

This project maintains high code quality standards:

- **Rust**: Uses Clippy for linting and rustfmt for formatting
- **JavaScript/TypeScript**: Uses Prettier for consistent formatting
- **Git Hooks**: Pre-commit hooks ensure code quality

Check formatting:
```bash
yarn prettier:check
```

Fix formatting issues:
```bash
yarn prettier:fix
```

## 📄 License

This project is licensed under the MIT License - see the individual package LICENSE files for details.
