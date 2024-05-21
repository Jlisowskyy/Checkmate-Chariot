//
// Created by Jlisowskyy on 3/22/24.
//

#include "../include/MoveGeneration/MoveGenerator.h"

std::map<std::string, uint64_t> MoveGenerator::GetCountedMoves(const int depth)
{
    TraceIfFalse(depth >= 1, "Depth must be at least 1!");

    std::map<std::string, uint64_t> rv{};

    VolatileBoardData data{_board};
    auto moves = GetMovesFast<false, false>();
    for (size_t i = 0; i < moves.size; ++i)
    {
        Move::MakeMove(moves[i], _board);
        const uint64_t moveCount = CountMoves(depth - 1);
        Move::UnmakeMove(moves[i], _board, data);

        rv.emplace(moves[i].GetLongAlgebraicNotation(), moveCount);
    }

    _threadStack.PopAggregate(moves);
    return rv;
}

uint64_t MoveGenerator::CountMoves(const int depth)
{
    if (depth == 0)
        return 1;

    const auto moves = GetMovesFast<false, false>();

    if (depth == 1)
    {
        _threadStack.PopAggregate(moves);
        return moves.size;
    }

    uint64_t sum{};

    VolatileBoardData data{_board};
    for (size_t i = 0; i < moves.size; ++i)
    {
        Move::MakeMove(moves[i], _board);
        sum += CountMoves(depth - 1);
        Move::UnmakeMove(moves[i], _board, data);
    }

    _threadStack.PopAggregate(moves);
    return sum;
}
