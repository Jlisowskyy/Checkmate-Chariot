//
// Created by Jlisowskyy on 3/19/24.
//

#ifndef COUNTERMOVETABLE_H
#define COUNTERMOVETABLE_H

#include "../MoveGeneration/Move.h"
#include "../EngineUtils.h"

/*
 *  Class used to implement so-called counter-move heuristic.
 *  The idea is that some moves have natural response to be played, especially in close positions.
 *  Table is used to save non-capture moves that caused beta cut-offs in the past as a response to some move.
 *  In move generation, we will make counter-moves more important
 *
 *  Resources: https://www.chessprogramming.org/Countermove_Heuristic
 */

struct CounterMoveTable
{
    // ------------------------------
    // Class creation
    // ------------------------------

    constexpr CounterMoveTable()  = default;
    constexpr ~CounterMoveTable() = default;

    CounterMoveTable(CounterMoveTable &&)      = delete;
    CounterMoveTable(const CounterMoveTable &) = delete;

    CounterMoveTable &operator=(const CounterMoveTable &) = delete;
    CounterMoveTable &operator=(CounterMoveTable &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // Function returns move that is currently saved as counter move for that one
    [[nodiscard]] INLINE PackedMove GetCounterMove(const Move previousMove) const
    {
        return _counterMovesTable[previousMove.GetStartBoardIndex()][previousMove.GetTargetField()];
    }

    // Simply saves counter move for given move, without checking or considering anything
    INLINE void  SaveCounterMove(const PackedMove counterMove, const Move previousMove)
    {
        _counterMovesTable[previousMove.GetStartBoardIndex()][previousMove.GetTargetField()] = counterMove;
    }

    // Resets all values inside the table to default null move one
    void ClearTable();

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    PackedMove _counterMovesTable[Board::BitBoardsCount][Board::BitBoardFields]{};
};

#endif // COUNTERMOVETABLE_H
