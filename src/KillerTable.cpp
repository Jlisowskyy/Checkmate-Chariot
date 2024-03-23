//
// Created by Jlisowskyy on 3/23/24.
//

#include "../include/Evaluation/KillerTable.h"

void KillerTable::_killerFloor_t::Push(const Move mv)
{
    // all possible slots are used
    if (last == MovesPerPly) return;

    // ensuring that no same moves are stored twice
    for(size_t i = 0; i < last; ++i)
        if (_killerMovesTable[i] == mv.GetPackedMove()) return;

    // saving move
    _killerMovesTable[last++] = mv.GetPackedMove();
}

bool KillerTable::_killerFloor_t::Contains(const Move mv) const
{
    for (size_t i = 0; i < MovesPerPly; ++i)
        if (_killerMovesTable[i] == mv.GetPackedMove())
            return true;
    return false;
}
