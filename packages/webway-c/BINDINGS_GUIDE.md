# Custom C Bindings & Inter-Container Communication Guide

## 🔧 **Your Current Setup**

Your `webway-c` container now has comprehensive support for:

### **1. Custom C Bindings & FFI**
- **libffi** - Foreign Function Interface for calling functions dynamically
- **libdl** - Dynamic library loading (already in glibc)
- **protobuf-c** - Protocol Buffers for C

### **2. Redpanda/Kafka Integration** 
- **librdkafka** - High-performance Kafka client
- **libjson-c** - JSON parsing for message payloads
- **lz4, zlib** - Compression support

### **3. Inter-Container Communication**
- **libzmq** - ZeroMQ for high-performance messaging
- **libwebsockets** - WebSocket protocol support
- **libcurl** - HTTP/REST API client
- **libuv** - Async I/O event loop

### **4. Data Serialization**
- **libxml2** - XML parsing and generation  
- **libyaml** - YAML parsing and generation
- **libprotobuf-c** - Protocol Buffers

## 📋 **What "Custom C Bindings" Usually Means**

### **Scenario 1: Rust Library with C FFI**
If your Rust container exports functions via `extern "C"`:

```rust
// In your Rust container
#[no_mangle]
pub extern "C" fn process_data(input: *const u8, len: usize, output: *mut u8) -> i32 {
    // Rust processing logic
}
```

```c
// In your C code
#include "bindings_helper.h"

// Load the Rust shared library
void* rust_lib = load_rust_shared_library("/path/to/rust/library.so");

// Call the Rust function
int result = call_rust_function(rust_lib, "process_data", input_data, input_size, 
                               output_buffer, &output_size);
```

### **Scenario 2: Protocol/API Bindings**
```c
// HTTP API calls to your Rust container
http_response_t response;
int result = http_request_to_container(
    "http://webway-parser:8080/api/process",
    "POST",
    "Content-Type: application/json",
    "{\"data\": \"your_data\"}",
    &response
);

if (result == 0) {
    printf("Response: %s\n", response.data);
    free_http_response(&response);
}
```

### **Scenario 3: ZeroMQ Messaging**
```c
// High-performance messaging between containers
void* zmq_ctx = create_zmq_context();
void* socket = create_zmq_socket(zmq_ctx, ZMQ_REQ);
connect_to_rust_container(socket, "tcp://webway-parser:5555");

// Send data
send_to_rust_container(socket, data, data_size);

// Receive response
char buffer[1024];
zmq_recv(socket, buffer, sizeof(buffer), 0);
```

## 🔄 **Inter-Container Communication Options**

### **Option 1: Kafka/Redpanda (Already Working)**
```c
// Your existing setup - both containers publish/consume
kafka_send_automation_data(producer, data, "shared-topic");
```

### **Option 2: HTTP/REST APIs**
```c
// C container calls Rust container's HTTP API
http_request_to_container("http://webway-parser:8080/process", "POST", ...);
```

### **Option 3: ZeroMQ (High Performance)**
```c
// Direct socket communication
void* socket = create_zmq_socket(ctx, ZMQ_REQ);
zmq_connect(socket, "tcp://webway-parser:5555");
```

### **Option 4: Shared Volumes**
```yaml
# In docker-compose.yaml
volumes:
  - shared_data:/app/shared
```

## 🔧 **Adding Custom Bindings**

### **Step 1: If you have a .so file from another language**
1. Copy it to your container:
```dockerfile
COPY path/to/custom.so /usr/local/lib/
```

2. Load it in your C code:
```c
void* custom_lib = load_shared_library("/usr/local/lib/custom.so");
void* func = get_library_function(custom_lib, "your_function");
```

### **Step 2: If you need to call a REST API**
```c
http_response_t response;
http_request_to_container("http://external-service/api", "GET", NULL, NULL, &response);
```

### **Step 3: If you need Protocol Buffers**
```c
// Your AutomationData is already protobuf-compatible
uint8_t* buffer;
size_t size;
serialize_protobuf_c((ProtobufCMessage*)data, &buffer, &size);
```

## 🚀 **Connecting to Your Rust Container**

### **Method 1: HTTP API (Recommended for simple data exchange)**
Add to your Rust container:
```rust
// In Cargo.toml
[dependencies]
axum = "0.6"
tokio = { version = "1.0", features = ["full"] }

// HTTP server in Rust
#[tokio::main]
async fn main() {
    let app = Router::new()
        .route("/process", post(process_data_handler));
    
    axum::Server::bind(&"0.0.0.0:8080".parse().unwrap())
        .serve(app.into_make_service())
        .await
        .unwrap();
}
```

### **Method 2: ZeroMQ (Recommended for high-performance)**
Add to your Rust container:
```rust
// In Cargo.toml
[dependencies]
zmq = "0.9"

// ZeroMQ server in Rust  
let context = zmq::Context::new();
let responder = context.socket(zmq::REP).unwrap();
responder.bind("tcp://*:5555").unwrap();
```

### **Method 3: Kafka (You already have this working!)**
Both containers can publish/consume from shared topics.

## 📝 **Next Steps**

1. **Identify your binding type**: 
   - Shared library (.so file)?
   - REST API?
   - Message protocol?

2. **Choose communication method**:
   - HTTP for simple request/response
   - ZeroMQ for high-performance streaming
   - Kafka for async messaging (already working)

3. **Use the helper functions** in `bindings_helper.h` and `bindings_helper.c`

4. **Test with a simple example** first

## 🎯 **Available Helper Functions**

Your container now includes:
- `load_shared_library()` - Load .so files
- `http_request_to_container()` - HTTP client
- `create_zmq_context()` - ZeroMQ messaging
- `serialize_protobuf_c()` - Protocol buffer serialization
- `call_foreign_function()` - FFI function calls

All libraries are linked and ready to use! 🚀