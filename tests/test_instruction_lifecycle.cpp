#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(InstructionLifecycle, PreservesExpectedStageOrder) {
    std::vector<std::string> stages = {"Fetch", "Decode", "Dispatch", "Execute", "Retire"};

    ASSERT_EQ(stages.front(), "Fetch");
    ASSERT_EQ(stages.back(), "Retire");
    EXPECT_LT(
        std::find(stages.begin(), stages.end(), "Decode"),
        std::find(stages.begin(), stages.end(), "Execute")
    );
}
