#include <gtest/gtest.h>

#include "../include/Board.h"
#include "../include/EngineUtils.h"
#include "../include/ThreadManagement/GameTimeManager.h"

TEST(GameTimeManager, SearchManagerTimerDisabled)
{
    // Arrange
    GoTimeInfo tInfo;
    tInfo.wTime = 1000;
    tInfo.bTime = 1000;
    tInfo.moveTime = 1000;
    // Act
    // Assert
    ASSERT_DEATH(GameTimeManager::StartSearchManagementAsync(tInfo, Color::WHITE), "Timer must be running");
}

TEST(GameTimeManager, ManagerWithInfiniteTime)
{
    // Arrange
    GoTimeInfo tInfo = GoTimeInfo::GetInfiniteTime();
    // Act
    GameTimeManager::StartTimerAsync();
    // Assert
    ASSERT_DEATH(GameTimeManager::StartSearchManagementAsync(tInfo, Color::WHITE), "A finite time limit must be set");
}

TEST(GameTimeManager, ManagerWithFiniteTime)
{
    // Arrange
    GoTimeInfo tInfo;
    // Time for move set to 1 second
    tInfo.wTime = 1000;
    tInfo.bTime = 1000;
    tInfo.moveTime = 1000;

    // Act
    GameTimeManager::StartTimerAsync();
    GameTimeManager::StartSearchManagementAsync(tInfo, Color::WHITE);
    ASSERT_EQ(GameTimeManager::ShouldStop, false);
    // Wait for more than 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // Assert
    ASSERT_EQ(GameTimeManager::ShouldStop, true);
}
