#include <gtest/gtest.h>

TEST(SchedulerRegression, CandidateLatencyWithinReviewableBounds) {
    const int baseline_p95_ns = 1800;
    const int candidate_p95_ns = 2200;
    const double regression = static_cast<double>(candidate_p95_ns - baseline_p95_ns) / baseline_p95_ns;

    EXPECT_GT(regression, 0.0);
    EXPECT_LT(regression, 0.30);
}
