#include "kafka_producer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void delivery_report_callback(rd_kafka_t* rk, const rd_kafka_message_t* rkmessage, void* opaque) {
    (void)rk;
    (void)opaque;
    
    if (rkmessage->err) {
        fprintf(stderr, "❌ Message delivery failed: %s\n", rd_kafka_err2str(rkmessage->err));
    } else {
        printf("✅ Message delivered to partition %d at offset %ld\n",
               rkmessage->partition, rkmessage->offset);
    }
}

KafkaProducer* kafka_producer_create(const char* bootstrap_servers) {
    KafkaProducer* producer = malloc(sizeof(KafkaProducer));
    if (!producer) {
        return NULL;
    }
    
    char errstr[512];
    
    // Create configuration
    producer->conf = rd_kafka_conf_new();
    if (!producer->conf) {
        free(producer);
        return NULL;
    }
    
    // Set configuration properties
    if (rd_kafka_conf_set(producer->conf, "bootstrap.servers", bootstrap_servers, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set bootstrap.servers: %s\n", errstr);
        rd_kafka_conf_destroy(producer->conf);
        free(producer);
        return NULL;
    }
    
    // Set message size and compression
    rd_kafka_conf_set(producer->conf, "message.max.bytes", "10485760", errstr, sizeof(errstr)); // 10MB
    rd_kafka_conf_set(producer->conf, "compression.type", "lz4", errstr, sizeof(errstr));
    rd_kafka_conf_set(producer->conf, "batch.size", "1048576", errstr, sizeof(errstr)); // 1MB
    rd_kafka_conf_set(producer->conf, "linger.ms", "10", errstr, sizeof(errstr));
    
    // Set delivery report callback
    rd_kafka_conf_set_dr_msg_cb(producer->conf, delivery_report_callback);
    
    // Create producer instance
    producer->producer = rd_kafka_new(RD_KAFKA_PRODUCER, producer->conf, errstr, sizeof(errstr));
    if (!producer->producer) {
        fprintf(stderr, "Failed to create producer: %s\n", errstr);
        free(producer);
        return NULL;
    }
    
    producer->topic = NULL; // Will be set per-message
    
    return producer;
}

int kafka_create_topic_if_not_exists(const char* bootstrap_servers, const char* topic_name) {
    // For simplicity, we'll skip the admin client topic creation in this example
    // In a real implementation, you'd use rd_kafka_admin_* functions
    (void)bootstrap_servers; // Suppress unused parameter warning
    printf("📋 Topic '%s' assumed to exist or will be auto-created\n", topic_name);
    return 0;
}

int kafka_send_automation_data(KafkaProducer* producer, const AutomationData* data, const char* topic_name) {
    if (!producer || !data || !topic_name) {
        return -1;
    }
    
    // Serialize data
    uint8_t* buffer;
    size_t size;
    if (automation_data_serialize(data, &buffer, &size) != 0) {
        fprintf(stderr, "❌ Failed to serialize AutomationData\n");
        return -1;
    }
    
    // Print message size info
    double size_mb = (double)size / (1024.0 * 1024.0);
    printf("📦 Message %d - Serialized size: %zu bytes (%.2f MB)\n", 
           data->sequence_number, size, size_mb);
    
    // Create key string
    char key[32];
    snprintf(key, sizeof(key), "%d", data->sequence_number);
    
    // Send message
    int result = rd_kafka_producev(
        producer->producer,
        RD_KAFKA_V_TOPIC(topic_name),
        RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
        RD_KAFKA_V_KEY(key, strlen(key)),
        RD_KAFKA_V_VALUE(buffer, size),
        RD_KAFKA_V_OPAQUE(NULL),
        RD_KAFKA_V_END);
    
    if (result != 0) {
        fprintf(stderr, "❌ Failed to produce message: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
        free(buffer);
        return -1;
    }
    
    printf("✅ Message %d queued for delivery\n", data->sequence_number);
    
    // Poll for delivery reports
    rd_kafka_poll(producer->producer, 0);
    
    free(buffer);
    return 0;
}

int kafka_producer_flush(KafkaProducer* producer, int timeout_ms) {
    if (!producer) {
        return -1;
    }
    
    return rd_kafka_flush(producer->producer, timeout_ms);
}

void kafka_producer_free(KafkaProducer* producer) {
    if (producer) {
        if (producer->topic) {
            rd_kafka_topic_destroy(producer->topic);
        }
        if (producer->producer) {
            rd_kafka_destroy(producer->producer);
        }
        free(producer);
    }
}