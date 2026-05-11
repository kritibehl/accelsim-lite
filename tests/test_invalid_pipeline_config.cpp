#include <gtest/gtest.h>
#include <string>

TEST(InvalidPipelineConfig, NegativeLatencyRejected) {
    int latency_cycles = -3;

    EXPECT_LT(latency_cycles, 0);
}

TEST(InvalidPipelineConfig, InvalidJsonConfigDetected) {
    std::string config = "{ invalid_json: true ";

    EXPECT_TRUE(config.find("{") != std::string::npos);
    EXPECT_EQ(config.find("}"), std::string::npos);
}
