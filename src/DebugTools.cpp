//
// Created by Jlisowskyy on 5/31/24.
//

#include "../include/TestsAndDebugging/DebugTools.h"
#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/Search/ZobristHash.h"

bool IsDrawDebug(const Board &bd)
{
    MoveGenerator::stck s{};
    MoveGenerator generator{bd, s};

    auto mvs         = generator.GetMovesFast();
    const size_t cnt = mvs.size;
    s.PopAggregate(mvs);
    const uint64_t hash = ZHasher.GenerateHash(bd);

    return generator.IsDrawByReps(hash) || (cnt == 0 && !generator.IsCheck());
}

Move GetMoveDebug(const Board &bd, const std::string &str)
{
    Stack<Move, DEFAULT_STACK_SIZE> s;
    MoveGenerator mech{bd, s};
    auto moves = mech.GetMovesFast();

    for (size_t i = 0; i < moves.size; ++i)
        if (moves.data[i].GetLongAlgebraicNotation() == str)
        {
            s.PopAggregate(moves);
            return moves.data[i];
        }

    s.PopAggregate(moves);
    return {};
}

void AspWinStat::DisplayAndClean()
{
    const size_t total = _fails.size();

    GlobalLogger.LogStream << std::format("[ INFO ][ Asp Win Stats: {} ] ", total);
    while (!_fails.empty())
    {
        const auto [type, alpa, beta, eval] = _fails.front();
        _fails.pop();

        GlobalLogger.LogStream << std::format(
            "{} a:{}, b:{}, e:{} ",
            type == FailType::FailHigh  ? 'H'
            : type == FailType::FailLow ? 'L'
                                        : 'E',
            alpa, beta, eval
        );
    }
    GlobalLogger.LogStream << std::endl;
}

#ifdef __unix__

size_t StackStartAddress;

#endif