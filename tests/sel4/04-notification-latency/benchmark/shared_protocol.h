#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H

#include <stdint.h>

#define CHANNEL_ID 1
#define TEST_COUNT 3
#define MAX_PAYLOAD 1518

typedef struct {
    uint32_t sequence;
    uint32_t length;
    uint8_t payload[MAX_PAYLOAD];
} shared_packet_t;

/* Generate predictable data for integrity testing. */
static inline uint8_t expected_byte(uint32_t sequence,
                                    uint32_t index)
{
    return (uint8_t)((sequence * 31u +
                      index * 17u + 90u) & 0xffu);
}

#endif
