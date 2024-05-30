//
// Created by Jlisowskyy on 5/20/24.
//

#ifndef CHECKMATE_CHARIOT_DEBUGTOOLS_H
#define CHECKMATE_CHARIOT_DEBUGTOOLS_H

#include <queue>
#include <tuple>

#include "../CompilationConstants.h"
#include "../Interface/Logger.h"
#include "../MoveGeneration/MoveGenerator.h"
#include "../Search/ZobristHash.h"

/*
 * struct defines statistics gathered during execution about aspiration window flow
 * */

class AspWinStat
{
    enum class FailType
    {
        FailLow,
        FailHigh,
        FinalBoundaries
    };

    std::queue<std::tuple<FailType, int, int, int>> _fails{};

    public:
    void RetryFailLow(int alpha, int beta, int eval) { _fails.emplace(FailType::FailLow, alpha, beta, eval); }
    void RetryFailHigh(int alpha, int beta, int eval) { _fails.emplace(FailType::FailHigh, alpha, beta, eval); }
    void RecordFinalBoundaries(int alpha, int beta, int eval)
    {
        _fails.emplace(FailType::FinalBoundaries, alpha, beta, eval);
    }

    void DisplayAndClean()
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
};

inline bool IsDrawExtremelySlow(const Board &bd)
{
    MoveGenerator::stck s{};
    MoveGenerator generator{bd, s};

    auto mvs         = generator.GetMovesFast();
    const size_t cnt = mvs.size;
    s.PopAggregate(mvs);
    const uint64_t hash = ZHasher.GenerateHash(bd);

    return generator.IsDrawByReps(hash) || (cnt == 0 && !generator.IsCheck());
}

#endif // CHECKMATE_CHARIOT_DEBUGTOOLS_H
