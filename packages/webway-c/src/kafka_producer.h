#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <librdkafka/rdkafka.h>
#include "automation_data.h"

typedef struct {
    rd_kafka_t* producer;
    rd_kafka_conf_t* conf;
    rd_kafka_topic_t* topic;
} KafkaProducer;

/**
 * Create a new Kafka producer
 * @param bootstrap_servers Kafka bootstrap servers (e.g., "localhost:19092")
 * @return Pointer to KafkaProducer or NULL on error
 */
KafkaProducer* kafka_producer_create(const char* bootstrap_servers);

/**
 * Create a Kafka topic if it doesn't exist
 * @param bootstrap_servers Kafka bootstrap servers
 * @param topic_name Name of the topic to create
 * @return 0 on success, -1 on error
 */
int kafka_create_topic_if_not_exists(const char* bootstrap_servers, const char* topic_name);

/**
 * Send AutomationData to Kafka topic
 * @param producer Pointer to KafkaProducer
 * @param data Pointer to AutomationData to send
 * @param topic_name Name of the topic
 * @return 0 on success, -1 on error
 */
int kafka_send_automation_data(KafkaProducer* producer, const AutomationData* data, const char* topic_name);

/**
 * Free Kafka producer resources
 * @param producer Pointer to KafkaProducer to free
 */
void kafka_producer_free(KafkaProducer* producer);

/**
 * Flush pending messages and wait for completion
 * @param producer Pointer to KafkaProducer
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, -1 on error
 */
int kafka_producer_flush(KafkaProducer* producer, int timeout_ms);

#endif // KAFKA_PRODUCER_H