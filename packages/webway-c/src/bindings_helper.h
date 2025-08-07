#ifndef BINDINGS_HELPER_H
#define BINDINGS_HELPER_H

#include <ffi.h>
#include <dlfcn.h>
#include <stdint.h>
#include <zmq.h>
#include <nanomsg/nn.h>
#include <nanomsg/tcp.h>
#include <uv.h>
#include <curl/curl.h>
#include <protobuf-c/protobuf-c.h>
#include <json-c/json.h>

// ========================================
// FFI (Foreign Function Interface) Helpers
// ========================================

/**
 * Load a shared library (.so file) dynamically
 * @param library_path Path to the shared library
 * @return Handle to the library or NULL on error
 */
void* load_shared_library(const char* library_path);

/**
 * Get a function pointer from a loaded library
 * @param lib_handle Handle from load_shared_library
 * @param function_name Name of the function to load
 * @return Function pointer or NULL on error
 */
void* get_library_function(void* lib_handle, const char* function_name);

/**
 * Call a foreign function with FFI
 * @param func_ptr Function pointer
 * @param return_type FFI type for return value
 * @param arg_types Array of FFI types for arguments
 * @param arg_count Number of arguments
 * @param args Array of argument pointers
 * @param result Pointer to store result
 * @return 0 on success, -1 on error
 */
int call_foreign_function(void* func_ptr, ffi_type* return_type, 
                         ffi_type** arg_types, int arg_count, 
                         void** args, void* result);

// ========================================
// Inter-Container Communication
// ========================================

/**
 * Create a ZeroMQ context for high-performance messaging
 * @return ZMQ context or NULL on error
 */
void* create_zmq_context(void);

/**
 * Create a ZeroMQ socket for communication
 * @param context ZMQ context
 * @param socket_type Type of socket (ZMQ_REQ, ZMQ_REP, etc.)
 * @return ZMQ socket or NULL on error
 */
void* create_zmq_socket(void* context, int socket_type);

/**
 * Connect to your Rust container via ZeroMQ
 * @param socket ZMQ socket
 * @param rust_container_endpoint Endpoint like "tcp://rust-container:5555"
 * @return 0 on success, -1 on error
 */
int connect_to_rust_container(void* socket, const char* rust_container_endpoint);

/**
 * Send data to Rust container
 * @param socket ZMQ socket
 * @param data Data to send
 * @param size Size of data
 * @return 0 on success, -1 on error
 */
int send_to_rust_container(void* socket, const void* data, size_t size);

// ========================================
// Redpanda/Kafka Enhanced Integration
// ========================================

/**
 * Enhanced Kafka producer with custom configurations
 * @param brokers Kafka broker list
 * @param custom_config Custom configuration string
 * @return Producer handle or NULL on error
 */
void* create_enhanced_kafka_producer(const char* brokers, const char* custom_config);

/**
 * Send binary data to Redpanda with custom headers
 * @param producer Kafka producer
 * @param topic Topic name
 * @param key Message key
 * @param data Binary data
 * @param size Data size
 * @param headers Custom headers (JSON string)
 * @return 0 on success, -1 on error
 */
int send_binary_to_redpanda(void* producer, const char* topic, const char* key,
                           const void* data, size_t size, const char* headers);

// ========================================
// Protocol Buffer Helpers
// ========================================

/**
 * Serialize data to Protocol Buffers C format
 * @param message Protocol buffer message
 * @param buffer Output buffer (caller must free)
 * @param size Output size
 * @return 0 on success, -1 on error
 */
int serialize_protobuf_c(const ProtobufCMessage* message, uint8_t** buffer, size_t* size);

/**
 * Deserialize Protocol Buffers C format
 * @param descriptor Message descriptor
 * @param buffer Input buffer
 * @param size Buffer size
 * @return Deserialized message or NULL on error
 */
ProtobufCMessage* deserialize_protobuf_c(const ProtobufCMessageDescriptor* descriptor,
                                        const uint8_t* buffer, size_t size);

// ========================================
// HTTP/REST API Client for Container Communication
// ========================================

typedef struct {
    char* data;
    size_t size;
} http_response_t;

/**
 * Make HTTP request to another container
 * @param url Full URL (e.g., "http://rust-container:8080/api/data")
 * @param method HTTP method ("GET", "POST", etc.)
 * @param headers JSON string of headers
 * @param body Request body (can be NULL)
 * @param response Output response structure
 * @return 0 on success, -1 on error
 */
int http_request_to_container(const char* url, const char* method, 
                             const char* headers, const char* body,
                             http_response_t* response);

/**
 * Free HTTP response
 * @param response Response to free
 */
void free_http_response(http_response_t* response);

// ========================================
// Dynamic Library Loading Examples
// ========================================

/**
 * Example: Load a Rust-generated shared library
 * @param rust_lib_path Path to .so file from Rust crate
 * @return Library handle or NULL on error
 */
void* load_rust_shared_library(const char* rust_lib_path);

/**
 * Example: Call a Rust function via FFI
 * @param lib_handle Handle to Rust library
 * @param function_name Name of exported Rust function
 * @param input_data Input data
 * @param input_size Size of input
 * @param output_data Output buffer
 * @param output_size Output size
 * @return 0 on success, -1 on error
 */
int call_rust_function(void* lib_handle, const char* function_name,
                      const void* input_data, size_t input_size,
                      void* output_data, size_t* output_size);

#endif // BINDINGS_HELPER_H