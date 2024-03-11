//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/Search/BestMoveSearch.h"

void BestMoveSearch::_insertionSort(std::vector<std::pair<int, int>>& list)
{
    for (size_t i = 2; i < list.size(); ++i)
    {
        size_t j = i - 1;
        const auto val = list[i];
        while(list[j].second < val.second)
        {
            list[j+1] = list[j];
            j--;
        }
        list[j+1] = val;
    }
}

void BestMoveSearch::_heapSortMoves(MoveGenerator::payload moves)
{
    for (ssize_t i = 1; i < static_cast<ssize_t>(moves.size); ++i)
    {
        ssize_t j = i - 1;
        const auto val = moves[i];
        while(j >= 0 && moves[j].GetEval() < val.GetEval())
        {
            moves.data[j+1] = moves[j];
            j--;
        }
        moves.data[j+1] = val;
    }
}
