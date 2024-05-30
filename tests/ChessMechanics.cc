#include <gtest/gtest.h>

#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/TestsAndDebugging/TestSetup.h"
#include "../include/TestsAndDebugging/DebugTools.h"

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