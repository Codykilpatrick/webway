# Use the official Rust image as the base
FROM rust:1.75 as builder

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy the Cargo.toml and Cargo.lock files
COPY Cargo.toml Cargo.lock ./

# Copy the source code
COPY src/ ./src/

# Build the application in release mode
RUN cargo build --release

# Use a smaller base image for the final stage
FROM debian:bookworm-slim

# Install any runtime dependencies if needed
RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root user
RUN useradd -m -u 1000 appuser

# Set the working directory
WORKDIR /app

# Copy the compiled binary from the builder stage
COPY --from=builder /usr/src/app/target/release/webway-parser /app/webway-parser

# Create directory for test data
RUN mkdir -p /app/test_data

# Change ownership of the app directory to the non-root user
RUN chown -R appuser:appuser /app

# Switch to the non-root user
USER appuser

# Set the default command
CMD ["./webway-parser"]