//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/Search/BestMoveSearch.h"

void BestMoveSearch::_embeddedMoveSort(MoveGenerator::payload moves, const size_t range)
{
    for (ssize_t i = 1; i < static_cast<ssize_t>(range); ++i)
    {
        ssize_t j = i - 1;
        const auto val = moves[i];
        while (j >= 0 && moves[j].GetEval() < val.GetEval())
        {
            moves.data[j + 1] = moves[j];
            j--;
        }
        moves.data[j + 1] = val;
    }
}

void BestMoveSearch::_pullMoveToFront(MoveGenerator::payload moves, const Move mv)
{
    // preparing sentinel
    const Move sentinelOld = moves.data[moves.size];
    moves.data[moves.size] = mv;

    // finding stopping index
    size_t ind = 0;
    while(moves.data[ind] != mv) ind++;

    // replacing old element
    moves.data[moves.size] = sentinelOld;

    // if move found swapping
    if (ind != moves.size)
    {
        // moving moves to preserve sorted order
        while (ind != 0)
        {
            moves.data[ind] = moves.data[ind - 1];
            ind--;
        }
        moves.data[0] = mv;
    }
}

int BestMoveSearch::_getMateValue(const int depthLeft) const
{
    const int distToRoot = _currRootDepth - depthLeft;
    return NegativeInfinity + distToRoot;
}
