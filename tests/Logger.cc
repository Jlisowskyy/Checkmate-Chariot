#include <gtest/gtest.h>

#include "../include/Interface/Logger.h"

TEST(Logger, LogSingle)
{
    // Arrange
    std::stringstream ss;
    Logger logger(ss);

    // act
    logger.LogStream << "Hello" << ", World!";
    // assert
    std::string expected = "Hello, World!";
    ASSERT_EQ(ss.str(), expected);
}

TEST(Logger, LogInChain)
{
    // Arrange
    std::stringstream ss1;
    std::stringstream ss2;
    std::stringstream ss3;

    Logger logger1(ss1);
    auto *logger2 = new Logger(ss2);
    auto *logger3 = new Logger(ss3);

    logger1.SetNext(logger2).SetNext(logger3);

    // act
    logger1.LogStream << "Hello" << ", World!";
    // assert
    std::string expected = "Hello, World!";
    ASSERT_EQ(ss1.str(), expected);
    ASSERT_EQ(ss2.str(), expected);
    ASSERT_EQ(ss3.str(), expected);
}

TEST(Logger, LogAppendNext)
{
    // Arrange
    std::stringstream ss1;
    std::stringstream ss2;
    std::stringstream ss3;

    Logger logger1(ss1);
    auto *logger2 = new Logger(ss2);
    auto *logger3 = new Logger(ss3);

    logger1.AppendNext(logger2);
    logger1.AppendNext(logger3);

    // act
    logger1.LogStream << "Hello" << ", World!";
    // assert
    std::string expected = "Hello, World!";
    ASSERT_EQ(ss1.str(), expected);
    ASSERT_EQ(ss2.str(), expected);
    ASSERT_EQ(ss3.str(), expected);
}

TEST(Logger, MoveConstructorOperator)
{
    // Arrange
    std::stringstream ss1;
    std::stringstream ss2;
    std::stringstream ss3;

    Logger logger = std::move(Logger(ss1).AppendNext(new Logger(ss2)).AppendNext(new Logger(ss3)));
    // act
    logger.LogStream << "Hello" << ", World!";
    // assert
    std::string expected = "Hello, World!";
    ASSERT_EQ(ss1.str(), expected);
    ASSERT_EQ(ss2.str(), expected);
    ASSERT_EQ(ss3.str(), expected);
}

TEST(Logger, LogStream)
{
    // Arrange
    std::stringstream ss1;
    std::stringstream ss2;
    std::stringstream ss3;

    Logger logger = std::move(Logger(ss1).AppendNext(new Logger(ss2)).AppendNext(new Logger(ss3)));
    // act
    logger.LogStream << "Hello" << ", World!";
    // assert
    std::string expected = "Hello, World!";
    ASSERT_EQ(ss1.str(), expected);
    ASSERT_EQ(ss2.str(), expected);
    ASSERT_EQ(ss3.str(), expected);
}
TEST(Logger, TraceStream)
{
    // Arrange
    std::stringstream ss1;
    std::stringstream ss2;
    std::stringstream ss3;

    Logger logger = std::move(Logger(ss1).AppendNext(new Logger(ss2)).AppendNext(new Logger(ss3)));
    // act
    logger.TraceStream << "Hello" << ", World!";
    // assert
    std::string expected = "Hello, World!";
    ASSERT_EQ(ss1.str(), expected);
    ASSERT_EQ(ss2.str(), expected);
    ASSERT_EQ(ss3.str(), expected);
}
