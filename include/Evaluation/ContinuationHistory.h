//
// Created by Jlisowskyy on 7/15/24.
//

#ifndef CONTINUATION_HISTORY_H
#define CONTINUATION_HISTORY_H

#include "HistoricTable.h"

static constexpr size_t CHECK_COUNT = 2;
using ContinuationHistoryBase =
        StatTable<HISTORY_TABLE_POINTS_LIMIT, CONT_TABLE_SCALE_DOWN_FACTOR,
        CHECK_COUNT, Board::BitBoardsCount, Board::BitBoardFields, Board::BitBoardsCount, Board::BitBoardFields>;

class ContinuationHistory : ContinuationHistoryBase
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
        return (HistoricTable*)ContinuationHistoryBase::GetTable(isCheck, prevMove.GetStartBoardIndex(), prevMove.GetTargetField());
    }

    using ContinuationHistoryBase::ScaleTableDown;
    using ContinuationHistoryBase::ClearTable;

    // ------------------------------
    // Class fields
    // ------------------------------

    inline static HistoricTable DummyReadTable{};
    inline static HistoricTable DummyWriteTable{};
};

#endif // CONTINUATION_HISTORY_H
