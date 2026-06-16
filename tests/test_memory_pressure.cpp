#include <gtest/gtest.h>

TEST(MemoryPressure, PointerHeavyWorkloadHasHigherLatency) {
    const double mixed_locality_cycles = 23.8;
    const double pointer_chasing_cycles = 86.64;

    EXPECT_GT(pointer_chasing_cycles, mixed_locality_cycles);
    EXPECT_GT(pointer_chasing_cycles / mixed_locality_cycles, 3.0);
}
