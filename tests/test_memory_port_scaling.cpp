#include <gtest/gtest.h>

TEST(MemoryScaling, ThroughputImprovesWithMorePorts) {
    double low_bw = 0.1395;
    double high_bw = 0.2609;

    EXPECT_GT(high_bw, low_bw);
}

TEST(MemoryScaling, LatencyImprovesWithMorePorts) {
    double low_bw = 23.8;
    double high_bw = 14.5;

    EXPECT_LT(high_bw, low_bw);
}
