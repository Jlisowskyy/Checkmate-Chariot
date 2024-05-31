#include <gtest/gtest.h>

#include "../include/Interface/Logger.h"

#include "../include/Interface/FenTranslator.h"
#include "../include/TestsAndDebugging/TestSetup.h"

TEST(GoCommandTest, stopCommandResponse)
{
    GameTimeManager::StartTimerAsync();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    SearchThreadManager threadManager{};
    Board board = FenTranslator::GetDefault();
    const auto Case = [&]<typename CaseFuncT>(CaseFuncT func)
    {
        auto t1 = std::chrono::steady_clock::now();

        func();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        threadManager.Stop();

        while (threadManager.IsSearchOn())
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            auto t2 = std::chrono::steady_clock::now();

            std::chrono::duration<double> time_span =
                std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
            ASSERT_LT(time_span.count(), 2.0);
        }

        auto t2 = std::chrono::steady_clock::now();
        ASSERT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count(), 1010);
    };

    auto infiniteSearch = [&]()
    {
        threadManager.GoInfinite(board);
    };

    auto depthSearch = [&]()
    {
        GoInfo info{};
        info.depth = 100;

        threadManager.Go(board, info);
    };

    auto moveTimeSearch = [&]()
    {
        GoInfo info{};
        info.timeInfo.moveTime = 100000;

        threadManager.Go(board, info);
    };

    auto colTimeSearch = [&]()
    {
        GoInfo info{};
        info.timeInfo.wTime = 100000;
        info.timeInfo.bTime = 100000;

        threadManager.Go(board, info);
    };

    Case(colTimeSearch);
    board = FenTranslator::GetDefault();
    Case(infiniteSearch);
    board = FenTranslator::GetDefault();
    Case(depthSearch);
    board = FenTranslator::GetDefault();
    Case(moveTimeSearch);
}

TEST(GoCommandTest, timeSpentTest)
{
    SearchThreadManager threadManager{};
    Board board = FenTranslator::GetDefault();

    auto t1 = std::chrono::steady_clock::now();

    GoInfo info{};
    info.timeInfo.moveTime = 1200;

    threadManager.Go(board, info);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    threadManager.Stop();

    while (threadManager.IsSearchOn())
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        auto t2 = std::chrono::steady_clock::now();

        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        ASSERT_LT(time_span.count(), 2.0);
    }

    auto t2 = std::chrono::steady_clock::now();
    ASSERT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count(), 1200 * 1.1);
}

TEST(Aging, EngineAging)
{
    TestSetup setup{};

    setup.Initialize();

    setup.ProcessCommandSync("position startpos moves e2e4 e7e5");

    ASSERT_EQ(setup.GetEngine().GetAge(), 3);
    ASSERT_EQ(setup.GetEngine().GetUnderlyingBoardCopy().HalfMoves, 0);

    setup.ProcessCommandSync("position startpos moves e2e4 e7e5 a2a4 a7a5 h2h4");

    ASSERT_EQ(setup.GetEngine().GetAge(), 6);
    ASSERT_EQ(setup.GetEngine().GetUnderlyingBoardCopy().HalfMoves, 0);

    setup.ProcessCommandSync("position startpos moves e2e4 e7e5 a2a4 a7a5 h2h4 d7d5 b1c3");

    ASSERT_EQ(setup.GetEngine().GetAge(), 8);
    ASSERT_EQ(setup.GetEngine().GetUnderlyingBoardCopy().HalfMoves, 1);

    setup.ProcessCommandSync("position startpos moves e2e4 e7e5 a2a4 a7a5 h2h4 d7d5 b1c3 d5e4");

    ASSERT_EQ(setup.GetEngine().GetAge(), 9);
    ASSERT_EQ(setup.GetEngine().GetUnderlyingBoardCopy().HalfMoves, 0);
}