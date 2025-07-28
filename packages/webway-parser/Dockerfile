# Use the official Rust image as the base
FROM rust:1.87

# Install rustfmt and clippy components
RUN rustup component add rustfmt clippy

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

# Create directory for test data
RUN mkdir -p /app/test_data

# Set the default command
CMD ["./target/release/webway-parser"]