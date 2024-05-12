//
// Created by Wookie on 5/12/24.
//

#include <gtest/gtest.h>
#include <chrono>

#include "../include/ThreadManagement/GameTimeManager.h"
#include "../include/Interface/FenTranslator.h"

class CalcTimePerMoveTest : public ::testing::Test {
protected:
    friend class GameTimeManager;
    void SetUp() override {
        GameTimeManager::StartTimerAsync();
    }

    void TearDown() override {
        GameTimeManager::StopSearchManagement();
    }
};

TEST_F(CalcTimePerMoveTest, ExpectedMoves)
{
    // Arrange
    GoTimeInfo tInfo;
    tInfo.wTime    = 1000;
    tInfo.bTime    = 1000;
    tInfo.moveTime = 1000;

    Board board = FenTranslator::GetDefault();

    // Act

    const lli t1 = 10000;
    // 10000 ms left on the clock, initial board, no increment, no move age
    const lli s1 = GameTimeManager::CalculateTimeMsPerMove(board, t1, GoTimeInfo::Infinite, 0, 0);
    // value should be less than t1/100

    // Assert
    ASSERT_LT(s1, t1 / 100);

}