#include "automation_data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

AutomationData* automation_data_new(int32_t message_key, int32_t sequence_number) {
    AutomationData* data = malloc(sizeof(AutomationData));
    if (!data) {
        return NULL;
    }
    
    data->message_key = message_key;
    data->sequence_number = sequence_number;
    
    // Get current Unix timestamp
    struct timeval tv;
    gettimeofday(&tv, NULL);
    data->sys_timestamp = (uint64_t)tv.tv_sec;
    
    // Initialize random seed if not already done
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    // Generate 780,000 random floats for normalized data (between 0.0 and 1.0)
    for (int i = 0; i < NORMALIZED_DATA_SIZE; i++) {
        data->normalized_data[i] = (float)rand() / (float)RAND_MAX;
    }
    
    // Generate 780,000 random floats for unnormalized data (-1000.0 to 1000.0)
    for (int i = 0; i < UNNORMALIZED_DATA_SIZE; i++) {
        data->unnormalized_data[i] = ((float)rand() / (float)RAND_MAX) * 2000.0f - 1000.0f;
    }
    
    return data;
}

void automation_data_free(AutomationData* data) {
    if (data) {
        free(data);
    }
}

void automation_data_print_summary(const AutomationData* data) {
    if (!data) {
        printf("❌ AutomationData is NULL\n");
        return;
    }
    
    // Calculate raw data size
    size_t raw_data_size = (NORMALIZED_DATA_SIZE * sizeof(float)) + 
                          (UNNORMALIZED_DATA_SIZE * sizeof(float)) + 
                          sizeof(int32_t) + sizeof(int32_t) + sizeof(uint64_t);
    double raw_data_size_mb = (double)raw_data_size / (1024.0 * 1024.0);
    
    printf("\n🔄 AutomationData Summary\n");
    printf("   📊 Message Key: %d\n", data->message_key);
    printf("   🔢 Sequence Number: %d\n", data->sequence_number);
    printf("   ⏰ Timestamp: %lu\n", (unsigned long)data->sys_timestamp);
    printf("   📈 Normalized Data: %d floats\n", NORMALIZED_DATA_SIZE);
    printf("   📉 Unnormalized Data: %d floats\n", UNNORMALIZED_DATA_SIZE);
    printf("   📁 Raw Data Size: %zu bytes (%.2f MB)\n", raw_data_size, raw_data_size_mb);
}

// Simple protobuf-like serialization (for demonstration)
// In a real implementation, you'd use protobuf-c library
int automation_data_serialize(const AutomationData* data, uint8_t** buffer, size_t* size) {
    if (!data || !buffer || !size) {
        return -1;
    }
    
    // Calculate total size needed
    *size = sizeof(int32_t) + sizeof(int32_t) + sizeof(uint64_t) + 
            (NORMALIZED_DATA_SIZE * sizeof(float)) + 
            (UNNORMALIZED_DATA_SIZE * sizeof(float));
    
    *buffer = malloc(*size);
    if (!*buffer) {
        return -1;
    }
    
    uint8_t* ptr = *buffer;
    
    // Serialize fields in order
    memcpy(ptr, &data->message_key, sizeof(int32_t));
    ptr += sizeof(int32_t);
    
    memcpy(ptr, &data->sequence_number, sizeof(int32_t));
    ptr += sizeof(int32_t);
    
    memcpy(ptr, &data->sys_timestamp, sizeof(uint64_t));
    ptr += sizeof(uint64_t);
    
    memcpy(ptr, data->normalized_data, NORMALIZED_DATA_SIZE * sizeof(float));
    ptr += NORMALIZED_DATA_SIZE * sizeof(float);
    
    memcpy(ptr, data->unnormalized_data, UNNORMALIZED_DATA_SIZE * sizeof(float));
    
    return 0;
}

AutomationData* automation_data_deserialize(const uint8_t* buffer, size_t size) {
    if (!buffer || size < sizeof(AutomationData)) {
        return NULL;
    }
    
    AutomationData* data = malloc(sizeof(AutomationData));
    if (!data) {
        return NULL;
    }
    
    const uint8_t* ptr = buffer;
    
    // Deserialize fields in order
    memcpy(&data->message_key, ptr, sizeof(int32_t));
    ptr += sizeof(int32_t);
    
    memcpy(&data->sequence_number, ptr, sizeof(int32_t));
    ptr += sizeof(int32_t);
    
    memcpy(&data->sys_timestamp, ptr, sizeof(uint64_t));
    ptr += sizeof(uint64_t);
    
    memcpy(data->normalized_data, ptr, NORMALIZED_DATA_SIZE * sizeof(float));
    ptr += NORMALIZED_DATA_SIZE * sizeof(float);
    
    memcpy(data->unnormalized_data, ptr, UNNORMALIZED_DATA_SIZE * sizeof(float));
    
    return data;
}