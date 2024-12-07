#include <gtest/gtest.h>

#include <memory>

#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/TestsAndDebugging/DebugTools.h"
#include "../include/TestsAndDebugging/TestSetup.h"

TEST(ChessMechTests, Draw50Test)
{
    TestSetup setup{};

    setup.Initialize();

    setup.ProcessCommandSync("position fen 3r2k1/B7/4q3/1R4P1/1P3r2/8/2P2P1p/R4K2 w - - 49 49 moves b5b6");

    Board bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(IsDrawDebug(bd));

    setup.ProcessCommandSync("position fen 3r2k1/B7/4q3/1R4P1/1P3r2/8/2P2P1p/R4K2 w - - 49 49 moves c2c3");

    bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(!IsDrawDebug(bd));

    setup.ProcessCommandSync("position fen 3r2k1/Bp6/4q3/1R4P1/1P3r2/8/2P2P1p/R4K2 w - - 49 49 moves b5b7");

    bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(!IsDrawDebug(bd));
}

TEST(ChessMechTests, ThreeFoldRepetition)
{
    auto counter = [](std::unordered_map<uint64_t, int> &map) -> int
    {
        int sum{};
        for (auto [key, count] : map) sum += count;
        return sum;
    };

    TestSetup setup{};

    setup.Initialize();

    setup.ProcessCommandSync("position startpos moves g1f3 b8c6 f3g1 c6b8 g1f3 b8c6 f3g1 c6b8");
    Board bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(IsDrawDebug(bd));
    EXPECT_EQ(counter(bd.Repetitions), 9);

    setup.ProcessCommandSync("position fen rnbqkbnr/pppppppp/Q7/8/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1 moves b8c6 g1f3 "
                             "c6b8 f3g1 b8c6 g1f3 c6b8 f3g1");
    bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(IsDrawDebug(bd));
    EXPECT_EQ(counter(bd.Repetitions), 9);

    setup.ProcessCommandSync(
        "position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1 moves b8c6 g1f3 c6b8 f3g1 b8c6 g1f3 c6b8"
    );
    bd = setup.GetEngine().GetUnderlyingBoardCopy();
    EXPECT_TRUE(!IsDrawDebug(bd));
    EXPECT_EQ(counter(bd.Repetitions), 8);

    Stack<Move, DEFAULT_STACK_SIZE> s;
    auto searcher = std::make_shared<BestMoveSearch>(bd, s);

    EXPECT_EQ(searcher->IterativeDeepening(nullptr, nullptr, 5, false), 0);
}

TEST(ChessMechTests, SEE1)
{
    static const char *positions[]{
        "1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - - 0 1", "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1"
    };

    static const char *moves[]{"e1e5", "d3e5"};

    static const int scores[] = {100, -225};

    for (size_t i = 0; i < sizeof(positions) / sizeof(const char *); ++i)
    {
        const Board bd = FenTranslator::GetTranslated(positions[i]);
        ChessMechanics mech{bd};
        const Move mv = GetMoveDebug(bd, moves[i]);

        EXPECT_EQ(mech.SEE(mv), scores[i] / SCORE_GRAIN);
    }
}

TEST(ChessMechTests, SEE_OVERFLOW_TEST)
{
    static constexpr const char* pos = "2r5/3R4/8/3Rk3/3k4/K7/8/8 b - - 0 71";
    static constexpr const char* move = "d4d5";
    const Board bd = FenTranslator::GetTranslated(pos);
    const Move mv = GetMoveDebug(bd, move);

    ChessMechanics mech{bd};
    [[maybe_unused]] const auto _ = mech.SEE(mv);
}