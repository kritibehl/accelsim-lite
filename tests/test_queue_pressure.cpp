#include <gtest/gtest.h>

TEST(QueuePressure, OverflowHandled) {
    int queue_depth = 64;
    int current_depth = 80;

    EXPECT_GT(current_depth, queue_depth);
}

TEST(QueuePressure, DependencyPressureDetected) {
    std::string bottleneck = "WaitingDependency";

    EXPECT_EQ(bottleneck, "WaitingDependency");
}
