#include <gtest/gtest.h>

#include <vector>
#include <string>

#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/TestsAndDebugging/TestSetup.h"
#include "../include/TestsAndDebugging/DebugTools.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/Search/ZobristHash.h"

TEST(TranspositionTableTests, HashFunctionTest1)
{
//    return;

    std::vector<std::string> posCommand{
        "position fen r2q2k1/1ppbb3/p3pr2/1P1p2p1/3P2P1/P1NQ1N2/2P2P2/R3K2R w KQ - 0 18"
    };

    std::vector<std::vector<std::string>> movesSubCommands{
        {"d3h7", "g8f8", "h7h8", "f8f7"},
    };

    for (size_t i = 0; i < posCommand.size(); ++i)
    {
        const std::string& position = posCommand[i];
        const std::vector<std::string>& moves = movesSubCommands[i];

        TestSetup setup{};
        setup.Initialize();

        setup.ProcessCommandSync(position);
        uint64_t hash = ZHasher.GenerateHash(setup.GetEngine().GetUnderlyingBoardCopy());

        std::string mvSubCommand = " moves ";
        for(const std::string& mv : moves)
        {
            mvSubCommand += mv + ' ';
            const std::string fullCommand = position + mvSubCommand;

            const Board bd = setup.GetEngine().GetUnderlyingBoardCopy();
            const VolatileBoardData vd{bd};

            Move currMove = GetMoveDebug(bd, mv);
            EXPECT_EQ(currMove.GetLongAlgebraicNotation(), mv);

            setup.ProcessCommandSync(fullCommand);
            hash = ZHasher.UpdateHash(hash, currMove, vd);

            const uint64_t genHash = ZHasher.GenerateHash(setup.GetEngine().GetUnderlyingBoardCopy());

            // Generated hash should be same as normal hash
            EXPECT_EQ(hash, genHash);
        }
    }
}