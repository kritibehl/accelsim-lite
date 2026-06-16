#include "ring_buffer.h"

void fw_ring_buffer_init(fw_ring_buffer_t* rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

bool fw_ring_buffer_push(fw_ring_buffer_t* rb, uint8_t value) {
    if (fw_ring_buffer_is_full(rb)) {
        return false;
    }

    rb->data[rb->tail] = value;
    rb->tail = (rb->tail + 1) % FW_RING_BUFFER_CAPACITY;
    rb->count++;
    return true;
}

bool fw_ring_buffer_pop(fw_ring_buffer_t* rb, uint8_t* out) {
    if (fw_ring_buffer_is_empty(rb)) {
        return false;
    }

    *out = rb->data[rb->head];
    rb->head = (rb->head + 1) % FW_RING_BUFFER_CAPACITY;
    rb->count--;
    return true;
}

bool fw_ring_buffer_is_empty(const fw_ring_buffer_t* rb) {
    return rb->count == 0;
}

bool fw_ring_buffer_is_full(const fw_ring_buffer_t* rb) {
    return rb->count == FW_RING_BUFFER_CAPACITY;
}

size_t fw_ring_buffer_size(const fw_ring_buffer_t* rb) {
    return rb->count;
}
