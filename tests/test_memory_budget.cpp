#include <gtest/gtest.h>
#include "ring_buffer.h"
#include "fixed_point_filter.h"

TEST(MemoryBudget, RingBufferUsesFixedCapacity) {
    EXPECT_EQ(FW_RING_BUFFER_CAPACITY, 8);
    EXPECT_LE(sizeof(fw_ring_buffer_t), 32u);
}

TEST(MemoryBudget, FixedPointFilterStateIsSmall) {
    EXPECT_LE(sizeof(firmware_validation::FixedPointFilter), 8u);
}
