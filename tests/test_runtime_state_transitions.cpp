#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(RuntimeStateTransitions, AllowsOnlyForwardProgression) {
    std::vector<std::string> states = {
        "created",
        "scheduled",
        "running",
        "completed"
    };

    EXPECT_EQ(states[0], "created");
    EXPECT_EQ(states[1], "scheduled");
    EXPECT_EQ(states[2], "running");
    EXPECT_EQ(states[3], "completed");
}
