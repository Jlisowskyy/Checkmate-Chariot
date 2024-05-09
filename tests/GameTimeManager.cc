#include <gtest/gtest.h>

#include "../include/Board.h"
#include "../include/EngineUtils.h"
#include "../include/ThreadManagement/GameTimeManager.h"
#include "../include/Interface/FenTranslator.h"

TEST(GameTimeManager, SearchManagerTimerDisabled)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    // Arrange
    GoTimeInfo tInfo;
    tInfo.wTime    = 500;
    tInfo.bTime    = 500;
    tInfo.moveTime = 200;

    Board board = FenTranslator::GetDefault();

    // assert
    ASSERT_DEBUG_DEATH(GameTimeManager::StartSearchManagementAsync(tInfo, Color::WHITE, board), "Timer must be running");
}

TEST(GameTimeManager, ManagerWithFiniteTime)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    // Arrange
    GoTimeInfo tInfo;
    tInfo.wTime    = 1000;
    tInfo.bTime    = 1000;
    tInfo.moveTime = 1000;

    Board board = FenTranslator::GetDefault();

    // Act
    GameTimeManager::StartTimerAsync();
    GameTimeManager::StartSearchManagementAsync(tInfo, Color::WHITE, board);

    // Assert
    ASSERT_EQ(GameTimeManager::ShouldStop, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(1010));
    ASSERT_EQ(GameTimeManager::ShouldStop, true);
}
