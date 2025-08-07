#include "bindings_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========================================
// FFI (Foreign Function Interface) Helpers
// ========================================

void* load_shared_library(const char* library_path) {
    void* handle = dlopen(library_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load library %s: %s\n", library_path, dlerror());
        return NULL;
    }
    printf("✅ Loaded shared library: %s\n", library_path);
    return handle;
}

void* get_library_function(void* lib_handle, const char* function_name) {
    if (!lib_handle || !function_name) {
        return NULL;
    }
    
    // Clear any existing error
    dlerror();
    
    void* func_ptr = dlsym(lib_handle, function_name);
    char* error = dlerror();
    if (error) {
        fprintf(stderr, "Failed to get function %s: %s\n", function_name, error);
        return NULL;
    }
    
    printf("✅ Found function: %s\n", function_name);
    return func_ptr;
}

int call_foreign_function(void* func_ptr, ffi_type* return_type, 
                         ffi_type** arg_types, int arg_count, 
                         void** args, void* result) {
    if (!func_ptr) {
        return -1;
    }
    
    ffi_cif cif;
    ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, arg_count, return_type, arg_types);
    
    if (status != FFI_OK) {
        fprintf(stderr, "Failed to prepare FFI call: %d\n", status);
        return -1;
    }
    
    ffi_call(&cif, FFI_FN(func_ptr), result, args);
    return 0;
}

// ========================================
// Inter-Container Communication
// ========================================

void* create_zmq_context(void) {
    void* context = zmq_ctx_new();
    if (!context) {
        fprintf(stderr, "Failed to create ZMQ context\n");
        return NULL;
    }
    printf("✅ Created ZMQ context\n");
    return context;
}

void* create_zmq_socket(void* context, int socket_type) {
    if (!context) {
        return NULL;
    }
    
    void* socket = zmq_socket(context, socket_type);
    if (!socket) {
        fprintf(stderr, "Failed to create ZMQ socket\n");
        return NULL;
    }
    
    printf("✅ Created ZMQ socket (type: %d)\n", socket_type);
    return socket;
}

int connect_to_rust_container(void* socket, const char* rust_container_endpoint) {
    if (!socket || !rust_container_endpoint) {
        return -1;
    }
    
    int rc = zmq_connect(socket, rust_container_endpoint);
    if (rc != 0) {
        fprintf(stderr, "Failed to connect to %s: %s\n", rust_container_endpoint, zmq_strerror(errno));
        return -1;
    }
    
    printf("✅ Connected to Rust container: %s\n", rust_container_endpoint);
    return 0;
}

int send_to_rust_container(void* socket, const void* data, size_t size) {
    if (!socket || !data) {
        return -1;
    }
    
    int rc = zmq_send(socket, data, size, 0);
    if (rc == -1) {
        fprintf(stderr, "Failed to send data: %s\n", zmq_strerror(errno));
        return -1;
    }
    
    printf("✅ Sent %zu bytes to Rust container\n", size);
    return 0;
}

// ========================================
// HTTP/REST API Client
// ========================================

static size_t write_callback(void* contents, size_t size, size_t nmemb, http_response_t* response) {
    size_t total_size = size * nmemb;
    
    response->data = realloc(response->data, response->size + total_size + 1);
    if (!response->data) {
        return 0;
    }
    
    memcpy(&(response->data[response->size]), contents, total_size);
    response->size += total_size;
    response->data[response->size] = 0;
    
    return total_size;
}

int http_request_to_container(const char* url, const char* method, 
                             const char* headers, const char* body,
                             http_response_t* response) {
    if (!url || !method || !response) {
        return -1;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return -1;
    }
    
    // Initialize response
    response->data = malloc(1);
    response->size = 0;
    
    // Set basic options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    
    // Set method
    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        }
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        }
    }
    
    // Set headers if provided
    struct curl_slist* header_list = NULL;
    if (headers) {
        // Parse JSON headers and add them
        // For simplicity, assuming headers is a single header string
        header_list = curl_slist_append(header_list, headers);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    }
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    // Cleanup
    if (header_list) {
        curl_slist_free_all(header_list);
    }
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "HTTP request failed: %s\n", curl_easy_strerror(res));
        free(response->data);
        response->data = NULL;
        response->size = 0;
        return -1;
    }
    
    printf("✅ HTTP %s to %s completed (%zu bytes)\n", method, url, response->size);
    return 0;
}

void free_http_response(http_response_t* response) {
    if (response && response->data) {
        free(response->data);
        response->data = NULL;
        response->size = 0;
    }
}

// ========================================
// Protocol Buffer Helpers
// ========================================

int serialize_protobuf_c(const ProtobufCMessage* message, uint8_t** buffer, size_t* size) {
    if (!message || !buffer || !size) {
        return -1;
    }
    
    *size = protobuf_c_message_get_packed_size(message);
    *buffer = malloc(*size);
    if (!*buffer) {
        return -1;
    }
    
    protobuf_c_message_pack(message, *buffer);
    printf("✅ Serialized protobuf message (%zu bytes)\n", *size);
    return 0;
}

ProtobufCMessage* deserialize_protobuf_c(const ProtobufCMessageDescriptor* descriptor,
                                        const uint8_t* buffer, size_t size) {
    if (!descriptor || !buffer) {
        return NULL;
    }
    
    ProtobufCMessage* message = protobuf_c_message_unpack(descriptor, NULL, size, buffer);
    if (message) {
        printf("✅ Deserialized protobuf message\n");
    } else {
        fprintf(stderr, "Failed to deserialize protobuf message\n");
    }
    
    return message;
}

// ========================================
// Dynamic Library Loading Examples
// ========================================

void* load_rust_shared_library(const char* rust_lib_path) {
    printf("🦀 Loading Rust shared library: %s\n", rust_lib_path);
    return load_shared_library(rust_lib_path);
}

int call_rust_function(void* lib_handle, const char* function_name,
                      const void* input_data, size_t input_size,
                      void* output_data, size_t* output_size) {
    if (!lib_handle || !function_name) {
        return -1;
    }
    
    // Get the function pointer
    typedef int (*rust_function_t)(const void*, size_t, void*, size_t*);
    rust_function_t rust_func = (rust_function_t)get_library_function(lib_handle, function_name);
    
    if (!rust_func) {
        return -1;
    }
    
    // Call the Rust function
    int result = rust_func(input_data, input_size, output_data, output_size);
    
    if (result == 0) {
        printf("✅ Called Rust function '%s' successfully\n", function_name);
    } else {
        fprintf(stderr, "❌ Rust function '%s' returned error: %d\n", function_name, result);
    }
    
    return result;
}