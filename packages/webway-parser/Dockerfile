# Use the official Rust image as the base
FROM rust:1.87

# Install only the rustup components you need
RUN rustup component add rustfmt clippy llvm-tools-preview

# Install cargo-llvm-cov for test coverage
RUN cargo install cargo-llvm-cov

# Install system dependencies
RUN apt-get update && apt-get install -y \
    vim \
    libsasl2-dev \
    libssl-dev \
    pkg-config \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy dependency files and fetch everything
COPY Cargo.toml Cargo.lock ./
COPY README.md ./
RUN cargo fetch

# Copy source code (for when you want to build/test)
COPY src/ ./src/

# Set a shell as default command since you're keeping vim
CMD ["/bin/bash"]