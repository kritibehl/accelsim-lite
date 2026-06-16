#include <gtest/gtest.h>

TEST(BenchmarkThresholds, DetectsP95Regression) {
    const int baseline_p95_ns = 410;
    const int candidate_p95_ns = 620;
    const double regression_percent =
        100.0 * (candidate_p95_ns - baseline_p95_ns) / baseline_p95_ns;

    EXPECT_GT(regression_percent, 25.0);
}
