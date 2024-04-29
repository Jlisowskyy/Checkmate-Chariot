//
// Created by Jlisowskyy on 3/19/24.
//

#ifndef COUNTERMOVETABLE_H
#define COUNTERMOVETABLE_H

#include "../MoveGeneration/Move.h"

/*
 *  Class used to implement so-called counter-move heuristic.
 *  The idea is that some moves have natural response to be played, especially in close positions.
 *  Table is used to save non-capture moves that caused beta cut-offs in the past as a response to some move.
 *  In move generation, we will make counter-moves more important
 */

struct CounterMoveTable
{
    // ------------------------------
    // Class creation
    // ------------------------------

    constexpr CounterMoveTable() = default;
    constexpr ~CounterMoveTable() = default;

    CounterMoveTable(CounterMoveTable&&) = delete;
    CounterMoveTable(const CounterMoveTable&) = delete;

    CounterMoveTable& operator=(const CounterMoveTable&) = delete;
    CounterMoveTable& operator=(CounterMoveTable&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] PackedMove GetCounterMove(const Move previousMove) const
    {
        return _counterMovesTable[previousMove.GetStartBoardIndex()][previousMove.GetTargetField()];
    }

    void SaveCounterMove(const PackedMove counterMove, const Move previousMove)
    {
        _counterMovesTable[previousMove.GetStartBoardIndex()][previousMove.GetTargetField()] = counterMove;
    }

    void ClearTable()
    {
        for (size_t i = 0 ; i < Board::BitBoardsCount; ++i)
            for (size_t j = 0; j < Board::BitBoardFields; ++j)
                _counterMovesTable[i][j] = {};
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    PackedMove _counterMovesTable[Board::BitBoardsCount][Board::BitBoardFields]{};
};

#endif //COUNTERMOVETABLE_H
