# Use the official Rust image as the base
FROM rust:1.87

# Install rustfmt and clippy components
RUN rustup component add rustfmt clippy

# Install llvm-tools-preview for coverage
RUN rustup component add llvm-tools-preview

# Install cargo-llvm-cov for test coverage
RUN cargo install cargo-llvm-cov

# Install system dependencies for Rust build
RUN apt-get update && apt-get install -y \
    vim \
    libsasl2-dev \
    libssl-dev \
    pkg-config \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy the Cargo.toml and Cargo.lock files
COPY Cargo.toml Cargo.lock ./

# Copy the README.md file
COPY README.md ./

# Copy the source code
COPY src/ ./src/

# Build the application in release mode
RUN cargo build --release

# Create directory for test data and coverage reports
RUN mkdir -p /app/test_data /app/coverage

# Set the default command (can be overridden)
CMD ["./target/release/webway-parser"]