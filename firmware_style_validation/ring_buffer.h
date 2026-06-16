#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FW_RING_BUFFER_CAPACITY 8

typedef struct {
    uint8_t data[FW_RING_BUFFER_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
} fw_ring_buffer_t;

void fw_ring_buffer_init(fw_ring_buffer_t* rb);
bool fw_ring_buffer_push(fw_ring_buffer_t* rb, uint8_t value);
bool fw_ring_buffer_pop(fw_ring_buffer_t* rb, uint8_t* out);
bool fw_ring_buffer_is_empty(const fw_ring_buffer_t* rb);
bool fw_ring_buffer_is_full(const fw_ring_buffer_t* rb);
size_t fw_ring_buffer_size(const fw_ring_buffer_t* rb);

#ifdef __cplusplus
}
#endif
