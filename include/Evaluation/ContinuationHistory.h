//
// Created by Jlisowskyy on 7/15/24.
//

#ifndef CONTINUATIONHISTORY_H
#define CONTINUATIONHISTORY_H

#include "HistoricTable.h"

class ContinuationHistory
{
    // ------------------------------
    // Class inner types
    // ------------------------------

    struct _contFloor_t
    {
        INLINE HistoricTable& GetTable(const Move prevMove)
        {
            return _tables[prevMove.GetStartField()][prevMove.GetTargetField()];
        }

        private:
        HistoricTable _tables[Board::BitBoardFields][Board::BitBoardFields];
    };
    // ------------------------------
    // Class creation
    // ------------------------------
    public:

    ContinuationHistory()=default;
    ~ContinuationHistory()=default;

    ContinuationHistory(const ContinuationHistory&) = delete;
    ContinuationHistory(ContinuationHistory&&) = delete;

    ContinuationHistory& operator=(const ContinuationHistory&) = delete;
    ContinuationHistory& operator=(ContinuationHistory&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    int GetScore(const int ply, const Move prevMove, const Move currMove)
    {
        return _floors[ply].GetTable(prevMove).GetBonusMove(currMove);
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    _contFloor_t _floors[MAX_SEARCH_DEPTH];
};

#endif //CONTINUATIONHISTORY_H
