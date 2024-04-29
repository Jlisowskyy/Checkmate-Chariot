//
// Created by Jlisowskyy on 4/29/24.
//

#include "../include/Evaluation/CounterMoveTable.h"
PackedMove CounterMoveTable::GetCounterMove(const Move previousMove) const
{
    return _counterMovesTable[previousMove.GetStartBoardIndex()][previousMove.GetTargetField()];
}
void CounterMoveTable::SaveCounterMove(const PackedMove counterMove, const Move previousMove)
{
    _counterMovesTable[previousMove.GetStartBoardIndex()][previousMove.GetTargetField()] = counterMove;
}
void CounterMoveTable::ClearTable()
{
    for (auto & from : _counterMovesTable)
        for (auto & fromTo : from) fromTo = PackedMove{};
}
