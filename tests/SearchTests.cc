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

    std::vector<std::string> posCommand
    {
        "position startpos"
    };

    std::vector<std::vector<std::string>> movesSubCommands{

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

            setup.ProcessCommandSync(fullCommand);

            const Board bd = setup.GetEngine().GetUnderlyingBoardCopy();
            VolatileBoardData vd{bd};

            Move currMove = GetMoveDebug(bd, mv);
            hash = ZHasher.UpdateHash(hash, currMove, vd);

            const uint64_t genHash = ZHasher.GenerateHash(bd);

            // Generated hash should be same as normal hash
            EXPECT_EQ(hash, genHash);
        }
    }
}