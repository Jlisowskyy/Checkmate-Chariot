#include <gtest/gtest.h>

#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/TestsAndDebugging/TestSetup.h"
#include "../include/TestsAndDebugging/DebugTools.h"
#include "../include/Search/BestMoveSearch.h"

TEST(ChessMechTests, Draw50Test)
{
    TestSetup setup{};

    setup.Initialize();

    setup.ProcessCommandSync("position fen 3r2k1/B7/4q3/1R4P1/1P3r2/8/2P2P1p/R4K2 w - - 49 49 moves b5b6");

    Board bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(IsDrawExtremelySlow(bd));

    setup.ProcessCommandSync("position fen 3r2k1/B7/4q3/1R4P1/1P3r2/8/2P2P1p/R4K2 w - - 49 49 moves c2c3");

    bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(!IsDrawExtremelySlow(bd));




    setup.ProcessCommandSync("position fen 3r2k1/Bp6/4q3/1R4P1/1P3r2/8/2P2P1p/R4K2 w - - 49 49 moves b5b7");

    bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(!IsDrawExtremelySlow(bd));
}

TEST(ChessMechTests, ThreeFoldRepetition)
{
    auto counter = [](std::unordered_map<uint64_t, int>& map) -> int
    {
        int sum{};
        for (auto [key, count] : map)
            sum += count;
        return sum;
    };

    TestSetup setup{};

    setup.Initialize();

    setup.ProcessCommandSync("position startpos moves g1f3 b8c6 f3g1 c6b8 g1f3 b8c6 f3g1 c6b8");
    Board bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(IsDrawExtremelySlow(bd));
    EXPECT_EQ(counter(bd.Repetitions), 9);

    setup.ProcessCommandSync("position fen rnbqkbnr/pppppppp/Q7/8/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1 moves b8c6 g1f3 c6b8 f3g1 b8c6 g1f3 c6b8 f3g1");
    bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(IsDrawExtremelySlow(bd));
    EXPECT_EQ(counter(bd.Repetitions), 9);

    setup.ProcessCommandSync("position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1 moves b8c6 g1f3 c6b8 f3g1 b8c6 g1f3 c6b8");
    bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(IsDrawExtremelySlow(bd));
    EXPECT_EQ(counter(bd.Repetitions), 8);

    Stack<Move, DEFAULT_STACK_SIZE> s;
    BestMoveSearch searcher{bd, s};

    EXPECT_EQ(searcher.IterativeDeepening(nullptr, nullptr, 5, false), 0);
}