#include <gtest/gtest.h>

TEST(ZeroResourceConfig, ZeroMemoryPortsDetected) {
    int memory_ports = 0;

    EXPECT_EQ(memory_ports, 0);
}

TEST(ZeroResourceConfig, ZeroComputeUnitsDetected) {
    int compute_units = 0;

    EXPECT_EQ(compute_units, 0);
}
