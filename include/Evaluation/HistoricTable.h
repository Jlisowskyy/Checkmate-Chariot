//
// Created by Jlisowskyy on 3/22/24.
//

#ifndef HISTORICTABLE_H
#define HISTORICTABLE_H

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"
#include "StatTable.h"

#include <cstring>

/*
 *  Class used to implement so-called history heuristic. The idea is based on the observation that during the search
 *  we are going to traverse boards that are usually similar to the ones we have already seen. Therefore, we can save
 *  best moves that were played in the past and use them to guide the search in the future.
 *
 *  Resources: https://www.chessprogramming.org/History_Heuristic
 */

using HistoricTableBase = StatTable<HISTORY_TABLE_POINTS_LIMIT, HISTORY_SCALE_DOWN_FACTOR, Board::BitBoardsCount, Board::BitBoardFields>;
struct HistoricTable : protected HistoricTableBase
{
    // ------------------------------
    // Class creation
    // ------------------------------

    HistoricTable() { ClearTable(); };

    ~HistoricTable() = default;

    HistoricTable(HistoricTable &&)      = delete;
    HistoricTable(const HistoricTable &) = delete;

    HistoricTable &operator=(const HistoricTable &) = delete;
    HistoricTable &operator=(HistoricTable &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // Function takes move and depth and increments the move's value in the table
    INLINE void SetBonusMove(const Move mv, const int depth)
    {
        SetPoints(PointScaleBonus(depth), mv.GetStartBoardIndex(), mv.GetStartField());
    }

    INLINE void SetPenaltyMove(const Move mv, const int depth)
    {
        SetPoints(PointScalePenalty(depth), mv.GetStartBoardIndex(), mv.GetStartField());
    }

    // Function returns the value of the move from the table
    [[nodiscard]] INLINE int32_t GetBonusMove(const Move mv) const
    {
        return Get(mv.GetStartBoardIndex(), mv.GetTargetField());
    }

    using HistoricTableBase::ClearTable;
    using HistoricTableBase::ScaleTableDown;

    // Debug function to display statistics about the content
    // Display average points value inside the table and 10 best saved 'moves'
    void DisplayStats() const;

    // Function used to determine the bonus value of the move, it is here to simplify its manipulation.
    INLINE static int PointScaleBonus (const int depth)
    {
        return HISTORY_BONUS_COEF::Get() * depth + HISTORY_BONUS_BIAS::Get();
    };

    INLINE static int PointScalePenalty(const int depth)
    {
        return -(HISTORY_PENALTY_COEF::Get() * depth / 2 + HISTORY_PENALTY_BIAS::Get());
    }
};

#endif // HISTORICTABLE_H
