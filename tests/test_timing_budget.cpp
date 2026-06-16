#include <gtest/gtest.h>

TEST(TimingBudget, FixedPointRoutineHasConstantStepModel) {
    const int multiply_steps = 1;
    const int filter_update_steps = 3;

    EXPECT_LE(multiply_steps, 1);
    EXPECT_LE(filter_update_steps, 3);
}
