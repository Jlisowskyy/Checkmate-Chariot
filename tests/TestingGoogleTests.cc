#include <gtest/gtest.h>
// A simple dummy test. Trying to run it automatically with github actions.

TEST(GithubIntegration, DummyTest) {
    EXPECT_EQ(1, 1) << "Github actions integration is being executed";
}