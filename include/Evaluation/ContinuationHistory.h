//
// Created by Jlisowskyy on 7/15/24.
//

#ifndef CONTINUATIONHISTORY_H
#define CONTINUATIONHISTORY_H

#include "HistoricTable.h"

class ContinuationHistory
{
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

    HistoricTable* GetTable(const Move prevMove, bool isCheck)
    {
        return &_tables[isCheck][prevMove.GetStartBoardIndex()][prevMove.GetTargetField()];
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    inline static HistoricTable DummyReadTable{};
    inline static HistoricTable DummyWriteTable{};
private:
    HistoricTable _tables[2][Board::BitBoardsCount][Board::BitBoardFields];
};

#endif //CONTINUATIONHISTORY_H
