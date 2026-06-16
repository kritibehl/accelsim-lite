#include <gtest/gtest.h>
#include "fixed_point_filter.h"

TEST(FixedPointFilter, MatchesReferenceOutputs) {
    const std::vector<int16_t> inputs = {256, 256, 512, 0};
    const std::vector<int16_t> expected = {128, 192, 352, 176};

    const auto outputs = firmware_validation::run_filter_sequence(inputs, 128);

    EXPECT_EQ(outputs, expected);
}

TEST(FixedPointFilter, Q88MultiplyIsBitExact) {
    EXPECT_EQ(firmware_validation::q8_8_mul(128, 256), 128);
    EXPECT_EQ(firmware_validation::q8_8_mul(256, 256), 256);
}
