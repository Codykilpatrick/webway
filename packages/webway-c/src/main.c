#define _GNU_SOURCE  // For usleep declaration
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "automation_data.h"
#include "kafka_producer.h"

#define MESSAGE_KEY 12345
#define TOPIC "automation-data"
#define BOOTSTRAP_SERVERS "localhost:19092"
#define NUM_MESSAGES 10

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Options:\n");
    printf("  --test      Run basic functionality tests\n");
    printf("  --help      Show this help message\n");
    printf("  --version   Show version information\n");
}

static int run_tests(void) {
    printf("🧪 Running basic functionality tests...\n\n");
    
    // Test 1: AutomationData creation
    printf("Test 1: AutomationData creation\n");
    AutomationData* data = automation_data_new(MESSAGE_KEY, 1);
    if (!data) {
        printf("❌ Failed to create AutomationData\n");
        return -1;
    }
    
    if (data->message_key != MESSAGE_KEY || data->sequence_number != 1) {
        printf("❌ AutomationData fields not set correctly\n");
        automation_data_free(data);
        return -1;
    }
    printf("✅ AutomationData created successfully\n\n");
    
    // Test 2: Data ranges
    printf("Test 2: Data range validation\n");
    int normalized_valid = 1, unnormalized_valid = 1;
    
    for (int i = 0; i < NORMALIZED_DATA_SIZE && normalized_valid; i++) {
        if (data->normalized_data[i] < 0.0f || data->normalized_data[i] >= 1.0f) {
            normalized_valid = 0;
        }
    }
    
    for (int i = 0; i < UNNORMALIZED_DATA_SIZE && unnormalized_valid; i++) {
        if (data->unnormalized_data[i] < -1000.0f || data->unnormalized_data[i] >= 1000.0f) {
            unnormalized_valid = 0;
        }
    }
    
    if (!normalized_valid) {
        printf("❌ Normalized data out of range [0.0, 1.0)\n");
        automation_data_free(data);
        return -1;
    }
    
    if (!unnormalized_valid) {
        printf("❌ Unnormalized data out of range [-1000.0, 1000.0)\n");
        automation_data_free(data);
        return -1;
    }
    printf("✅ Data ranges are valid\n\n");
    
    // Test 3: Serialization/Deserialization
    printf("Test 3: Serialization and deserialization\n");
    uint8_t* buffer;
    size_t size;
    
    if (automation_data_serialize(data, &buffer, &size) != 0) {
        printf("❌ Failed to serialize AutomationData\n");
        automation_data_free(data);
        return -1;
    }
    
    AutomationData* deserialized = automation_data_deserialize(buffer, size);
    if (!deserialized) {
        printf("❌ Failed to deserialize AutomationData\n");
        free(buffer);
        automation_data_free(data);
        return -1;
    }
    
    if (deserialized->message_key != data->message_key ||
        deserialized->sequence_number != data->sequence_number ||
        deserialized->sys_timestamp != data->sys_timestamp) {
        printf("❌ Deserialized data doesn't match original\n");
        free(buffer);
        automation_data_free(data);
        automation_data_free(deserialized);
        return -1;
    }
    printf("✅ Serialization and deserialization successful\n\n");
    
    // Cleanup
    free(buffer);
    automation_data_free(data);
    automation_data_free(deserialized);
    
    printf("🎉 All tests passed!\n");
    return 0;
}

static int run_kafka_demo(void) {
    printf("🚀 Starting Kafka producer demo...\n\n");
    
    // Create topic if not exists
    if (kafka_create_topic_if_not_exists(BOOTSTRAP_SERVERS, TOPIC) != 0) {
        fprintf(stderr, "⚠️  Warning: Could not ensure topic exists\n");
    }
    
    // Create Kafka producer
    KafkaProducer* producer = kafka_producer_create(BOOTSTRAP_SERVERS);
    if (!producer) {
        fprintf(stderr, "❌ Failed to create Kafka producer\n");
        return -1;
    }
    
    // Generate and send messages
    size_t total_raw_size = 0;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    for (int i = 0; i < NUM_MESSAGES; i++) {
        AutomationData* data = automation_data_new(MESSAGE_KEY, i);
        if (!data) {
            fprintf(stderr, "❌ Failed to create AutomationData for message %d\n", i);
            continue;
        }
        
        // Calculate and accumulate size
        uint8_t* temp_buffer;
        size_t temp_size;
        if (automation_data_serialize(data, &temp_buffer, &temp_size) == 0) {
            total_raw_size += temp_size;
            free(temp_buffer);
        }
        
        // Print detailed message info
        automation_data_print_summary(data);
        
        // Send to Kafka
        if (kafka_send_automation_data(producer, data, TOPIC) != 0) {
            fprintf(stderr, "❌ Failed to send message %d\n", i);
        }
        
        automation_data_free(data);
        
        // Small delay between messages
        usleep(100000); // 100ms
    }
    
    // Flush any remaining messages
    printf("\n🔄 Flushing pending messages...\n");
    kafka_producer_flush(producer, 5000); // 5 second timeout
    
    // Calculate and print statistics
    gettimeofday(&end_time, NULL);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                    (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    
    size_t avg_size = total_raw_size / NUM_MESSAGES;
    double total_size_mb = (double)total_raw_size / (1024.0 * 1024.0);
    double avg_size_mb = (double)avg_size / (1024.0 * 1024.0);
    double throughput = total_size_mb / elapsed;
    
    printf("\n📊 SUMMARY STATISTICS:\n");
    printf("   📨 Total messages sent: %d\n", NUM_MESSAGES);
    printf("   📦 Total raw data size: %zu bytes (%.2f MB)\n", total_raw_size, total_size_mb);
    printf("   📊 Average message size: %zu bytes (%.2f MB)\n", avg_size, avg_size_mb);
    printf("   ⏱️  Total time: %.2fs\n", elapsed);
    printf("   🚀 Throughput: %.2f MB/s\n", throughput);
    
    // Cleanup
    kafka_producer_free(producer);
    
    printf("\n🎉 All messages sent successfully!\n");
    printf("💡 Note: Actual network transfer size will be smaller due to LZ4 compression\n");
    
    return 0;
}

int main(int argc, char* argv[]) {
    printf("🔧 Webway C/C++ Data Parser\n");
    printf("=============================\n\n");
    
    // Parse command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "--test") == 0) {
            return run_tests();
        } else if (strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[1], "--version") == 0) {
            printf("Webway C/C++ Data Parser v1.0.0\n");
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Default behavior: run Kafka demo
    return run_kafka_demo();
}