//
// Created by Jlisowskyy on 7/15/24.
//

#ifndef CONTINUATIONHISTORY_H
#define CONTINUATIONHISTORY_H

#include "HistoricTable.h"

static constexpr size_t CHECK_COUNT = 2;
using ContinuationHistoryBase =
        StatTable<HISTORY_TABLE_POINTS_LIMIT, CONT_TABLE_SCALE_DOWN_FACTOR,
        CHECK_COUNT, Board::BitBoardsCount, Board::BitBoardFields, Board::BitBoardsCount, Board::BitBoardFields>;

class ContinuationHistory
{
    // ------------------------------
    // Class creation
    // ------------------------------
    public:

    ContinuationHistory() = default;
    ~ContinuationHistory() = default;

    ContinuationHistory(const ContinuationHistory&) = delete;
    ContinuationHistory(ContinuationHistory&&) = delete;

    ContinuationHistory& operator=(const ContinuationHistory&) = delete;
    ContinuationHistory& operator=(ContinuationHistory&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    HistoricTable* GetTable(const Move prevMove, const bool isCheck)
    {
        return &_tables[isCheck][prevMove.GetStartBoardIndex()][prevMove.GetTargetField()];
    }

    void ScaleDown() {
        for(auto& checks : _tables)
            for(auto& figs : checks)
                for (auto& tables : figs)
                    tables.ScaleTableDown();
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
