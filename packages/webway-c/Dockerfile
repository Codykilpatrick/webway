# Use Ubuntu as base image for C/C++ development
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools and development dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    clang \
    make \
    cmake \
    gdb \
    valgrind \
    vim \
    git \
    pkg-config \
    libssl-dev \
    libsasl2-dev \
    librdkafka-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libjson-c-dev \
    libcurl4-openssl-dev \
    zlib1g-dev \
    liblz4-dev \
    && rm -rf /var/lib/apt/lists/*

# Install additional libraries for FFI, bindings, and inter-container communication
RUN apt-get update && apt-get install -y \
    libffi-dev \
    libzmq3-dev \
    libuv1-dev \
    libev-dev \
    libevent-dev \
    libwebsockets-dev \
    libyaml-dev \
    libxml2-dev \
    libprotobuf-c-dev \
    protobuf-c-compiler \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install modern C++ libraries
RUN apt-get update && apt-get install -y \
    libfmt-dev \
    libspdlog-dev \
    catch2 \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy project files
COPY Makefile ./
COPY README.md ./

# Copy source code (for when you want to build/test)
COPY src/ ./src/

# Set environment variables for development
ENV CC=gcc
ENV CXX=g++
ENV CFLAGS="-Wall -Wextra -O2"
ENV CXXFLAGS="-Wall -Wextra -O2 -std=c++17"

# Set a shell as default command since you're keeping vim
CMD ["/bin/bash"]