#ifndef AUTOMATION_DATA_H
#define AUTOMATION_DATA_H

#include <stdint.h>
#include <time.h>

#define NORMALIZED_DATA_SIZE 780000
#define UNNORMALIZED_DATA_SIZE 780000

typedef struct {
    int32_t message_key;
    int32_t sequence_number;
    uint64_t sys_timestamp;
    float normalized_data[NORMALIZED_DATA_SIZE];
    float unnormalized_data[UNNORMALIZED_DATA_SIZE];
} AutomationData;

/**
 * Create a new AutomationData structure with random data
 * @param message_key The message key identifier
 * @param sequence_number The sequence number for this message
 * @return Pointer to newly created AutomationData (caller must free)
 */
AutomationData* automation_data_new(int32_t message_key, int32_t sequence_number);

/**
 * Free an AutomationData structure
 * @param data Pointer to the AutomationData to free
 */
void automation_data_free(AutomationData* data);

/**
 * Print summary information about the AutomationData
 * @param data Pointer to the AutomationData
 */
void automation_data_print_summary(const AutomationData* data);

/**
 * Serialize AutomationData to protobuf format
 * @param data Pointer to the AutomationData
 * @param buffer Pointer to buffer to store serialized data (caller must free)
 * @param size Pointer to store the size of serialized data
 * @return 0 on success, -1 on error
 */
int automation_data_serialize(const AutomationData* data, uint8_t** buffer, size_t* size);

/**
 * Deserialize AutomationData from protobuf format
 * @param buffer Pointer to serialized data
 * @param size Size of serialized data
 * @return Pointer to deserialized AutomationData (caller must free) or NULL on error
 */
AutomationData* automation_data_deserialize(const uint8_t* buffer, size_t size);

#endif // AUTOMATION_DATA_H