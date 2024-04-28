#include <gtest/gtest.h>
// A simple dummy test. Trying to run it automatically with github actions.

// Copied from: https://google.github.io/googletest/quickstart-cmake.html
// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}