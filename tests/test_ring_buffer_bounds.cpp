#include <gtest/gtest.h>
#include "ring_buffer.h"

TEST(RingBufferBounds, RejectsOverflowAndUnderflow) {
    fw_ring_buffer_t rb;
    fw_ring_buffer_init(&rb);

    uint8_t out = 0;
    EXPECT_FALSE(fw_ring_buffer_pop(&rb, &out));

    for (uint8_t i = 0; i < FW_RING_BUFFER_CAPACITY; ++i) {
        EXPECT_TRUE(fw_ring_buffer_push(&rb, i));
    }

    EXPECT_TRUE(fw_ring_buffer_is_full(&rb));
    EXPECT_FALSE(fw_ring_buffer_push(&rb, 99));

    for (uint8_t i = 0; i < FW_RING_BUFFER_CAPACITY; ++i) {
        EXPECT_TRUE(fw_ring_buffer_pop(&rb, &out));
        EXPECT_EQ(out, i);
    }

    EXPECT_TRUE(fw_ring_buffer_is_empty(&rb));
}
