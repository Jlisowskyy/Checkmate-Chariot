//
// Created by Wookie on 5/12/24.
//

#include <gtest/gtest.h>
<<<<<<< HEAD
=======
#include <chrono>
>>>>>>> 1ef9c0082a7962a7698dabdb223373bcdf8c3898

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

TEST_F(CalcTimePerMoveTest, ExpectedMovesAge)
{
    // Arrange
    GoTimeInfo tInfo;
    tInfo.wTime    = 1000;
    tInfo.bTime    = 1000;
    tInfo.moveTime = 1000;

    Board board = FenTranslator::GetDefault();
    FenTranslator::Translate("rn2k1nr/pp3ppp/2p5/2PN2B1/8/7b/PP3PPP/R3KBNR b KQkq - 5 11", board);

    const lli t1 = 10000;
    const lli t2 = 7546;
    const lli t3 = 1234;

    // Act
    // 10000 ms left on the clock, initial board, no increment, no move age
    const lli s1 = GameTimeManager::CalculateTimeMsPerMove(board, t1, GoTimeInfo::Infinite, 0, 0);
    // it know expects 100-0/2 = 50 moves, so the time should be less than t1/50
    const lli s2 = GameTimeManager::CalculateTimeMsPerMove(board, t1, GoTimeInfo::Infinite, 0, 50);
    // it know expects 50-50/2 = 25 moves, so the time should be less than t1/50
    const lli s3 = GameTimeManager::CalculateTimeMsPerMove(board, t1, GoTimeInfo::Infinite, 0, 70);
    // it know expects 100-70/2 = 15 moves, so the time should be less than t1/15
    const lli s4 = GameTimeManager::CalculateTimeMsPerMove(board, t2, GoTimeInfo::Infinite, 0, 0);
    // it know expects 100-0/2 = 50 moves, so the time should be less than t1/50
    const lli s5 = GameTimeManager::CalculateTimeMsPerMove(board, t2, GoTimeInfo::Infinite, 0, 50);
    // it know expects 50-50/2 = 25 moves, so the time should be less than t1/50
    const lli s6 = GameTimeManager::CalculateTimeMsPerMove(board, t2, GoTimeInfo::Infinite, 0, 70);
    // it know expects 100-70/2 = 15 moves, so the time should be less than t1/15
    const lli s7 = GameTimeManager::CalculateTimeMsPerMove(board, t3, GoTimeInfo::Infinite, 0, 0);
    // it know expects 100-0/2 = 50 moves, so the time should be less than t1/50
    const lli s8 = GameTimeManager::CalculateTimeMsPerMove(board, t3, GoTimeInfo::Infinite, 0, 50);
    // it know expects 50-50/2 = 25 moves, so the time should be less than t1/50
    const lli s9 = GameTimeManager::CalculateTimeMsPerMove(board, t3, GoTimeInfo::Infinite, 0, 70);


    // Assert
    // t1
    ASSERT_LT(s1, (t1 / 50)*1.1);
    ASSERT_LT(s2, (t1 / 25)*1.1);
    ASSERT_LT(s3, (t1 / 15)*1.1);
    // t2         (       )*1.1
    ASSERT_LT(s4, (t2 / 50)*1.1);
    ASSERT_LT(s5, (t2 / 25)*1.1);
    ASSERT_LT(s6, (t2 / 15)*1.1);
    // t3         (       )*1.1
    ASSERT_LT(s7, (t3 / 50)*1.1);
    ASSERT_LT(s8, (t3 / 25)*1.1);
    ASSERT_LT(s9, (t3 / 15)*1.1);
}
