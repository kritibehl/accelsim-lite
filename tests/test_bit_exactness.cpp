#include <gtest/gtest.h>
#include "fixed_point_filter.h"

TEST(BitExactness, ReferenceSequenceStaysStable) {
    const std::vector<int16_t> inputs = {256, 256, 512, 0};
    const std::vector<int16_t> expected = {128, 192, 352, 176};

    EXPECT_EQ(firmware_validation::run_filter_sequence(inputs, 128), expected);
}
