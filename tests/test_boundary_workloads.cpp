#include <gtest/gtest.h>
#include <vector>

TEST(BoundaryWorkloads, EmptyWorkloadHasNoCompletedOps) {
    std::vector<int> workload;

    EXPECT_TRUE(workload.empty());
    EXPECT_EQ(workload.size(), 0u);
}

TEST(BoundaryWorkloads, SingleInstructionWorkloadIsAccepted) {
    std::vector<int> workload = {1};

    EXPECT_EQ(workload.size(), 1u);
}
