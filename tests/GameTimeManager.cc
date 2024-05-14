#include <gtest/gtest.h>

#include "../include/Board.h"
#include "../include/EngineUtils.h"
#include "../include/Interface/FenTranslator.h"
#include "../include/ThreadManagement/GameTimeManager.h"

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
    ASSERT_DEBUG_DEATH(
        GameTimeManager::StartSearchManagementAsync(tInfo, Color::WHITE, board, 0), "Timer must be running"
    );
}