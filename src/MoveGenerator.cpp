//
// Created by Jlisowskyy on 3/22/24.
//

#include "../include/MoveGeneration/MoveGenerator.h"

std::map<std::string, uint64_t> MoveGenerator::GetCountedMoves(const int depth)
{
    std::map<std::string, uint64_t> rv{};

    const auto oldCastling = _board.Castlings;
    const auto oldElPassant = _board.elPassantField;

    auto moves = GetMovesFast<false, false>();
    for (size_t i = 0 ; i < moves.size; ++i)
    {
        Move::MakeMove(moves[i], _board);
        const uint64_t moveCount = CountMoves(depth - 1);
        Move::UnmakeMove(moves[i], _board, oldCastling, oldElPassant);

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

    const auto oldCastling = _board.Castlings;
    const auto oldElPassant = _board.elPassantField;

    for (size_t i = 0; i < moves.size; ++i)
    {
        Move::MakeMove(moves[i], _board);
        sum += CountMoves(depth - 1);
        Move::UnmakeMove(moves[i], _board, oldCastling, oldElPassant);
    }

    _threadStack.PopAggregate(moves);
    return sum;
}
