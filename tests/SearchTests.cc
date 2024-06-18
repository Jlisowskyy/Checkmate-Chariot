#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/ParseTools.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/Search/ZobristHash.h"
#include "../include/TestsAndDebugging/DebugTools.h"
#include "../include/TestsAndDebugging/TestSetup.h"

TEST(TranspositionTableTests, HashFunctionTest1)
{
    std::vector<std::string> posCommand{
        "position fen r2q2k1/1ppbb3/p3pr2/1P1p2p1/3P2P1/P1NQ1N2/2P2P2/R3K2R w KQ - 0 18", "position startpos"
    };

    std::vector<std::vector<std::string>> movesSubCommands{
        {"d3h7", "g8f8", "h7h8", "f8f7"},
        ParseTools::Split("g1f3 b7b6 e2e4 c8b7 b1c3 e7e6 a2a3 g8f6 e4e5 f6d5 f1c4 d5c3 d2c3 d7d6 c1g5 f8e7 g5e7 d8e7 "
                          "e5d6 c7d6 e1g1 e8g8"),
    };

    for (size_t i = 0; i < posCommand.size(); ++i)
    {
        const std::string &position           = posCommand[i];
        const std::vector<std::string> &moves = movesSubCommands[i];

        TestSetup setup{};
        setup.Initialize();

        setup.ProcessCommandSync(position);
        uint64_t hash = ZHasher.GenerateHash(setup.GetEngine().GetUnderlyingBoardCopy());

        std::string mvSubCommand = " moves ";
        for (const std::string &mv : moves)
        {
            mvSubCommand += mv + ' ';
            const std::string fullCommand = position + mvSubCommand;

            const Board bd = setup.GetEngine().GetUnderlyingBoardCopy();
            const VolatileBoardData vd{bd};

            Move currMove = GetMoveDebug(bd, mv);
            EXPECT_EQ(currMove.GetLongAlgebraicNotation(), mv);

            setup.ProcessCommandSync(fullCommand);

            if (mv == "e1g1")
                GlobalLogger.LogStream << "xd";

            const uint64_t genHash = ZHasher.GenerateHash(setup.GetEngine().GetUnderlyingBoardCopy());
            hash                   = ZHasher.UpdateHash(hash, currMove, vd);

            // Generated hash should be same as normal hash
            EXPECT_EQ(hash, genHash);

            if (hash != genHash)
                GlobalLogger.LogStream << "xd";
        }
    }
}