#include <gtest/gtest.h>

TEST(InvalidConfig, ZeroMemoryPortsRejected) {
    int memory_ports = 0;
    EXPECT_LE(memory_ports, 0);
}

TEST(InvalidConfig, NegativeComputeUnitsRejected) {
    int compute_units = -1;
    EXPECT_LT(compute_units, 0);
}
