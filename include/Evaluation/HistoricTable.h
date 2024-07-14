//
// Created by Jlisowskyy on 3/22/24.
//

#ifndef HISTORICTABLE_H
#define HISTORICTABLE_H

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"

#include <cstring>
#include <tuple>

/*
 *  Class used to implement so-called history heuristic. The idea is based on the observation that during the search
 *  we are going to traverse boards that are usually similar to the ones we have already seen. Therefore, we can save
 *  best moves that were played in the past and use them to guide the search in the future.
 *
 *  Resources: https://www.chessprogramming.org/History_Heuristic
 */

struct HistoricTable
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

    INLINE void SetPointsMove(const Move mv, const int points)
    {
        const int clampedPoints = std::clamp(points, -Barrier, Barrier);

        _table[mv.GetStartBoardIndex()][mv.GetTargetField()]
            += clampedPoints - _table[mv.GetStartBoardIndex()][mv.GetTargetField()] * abs(clampedPoints) / Barrier;
    }

    // Function takes move and depth and increments the move's value in the table
    INLINE void SetBonusMove(const Move mv, const int depth)
    {
        SetPointsMove(mv, _pointScaleBonus(depth));
    }

    INLINE void SetPenaltyMove(const Move mv, const int depth)
    {
        SetPointsMove(mv, _pointScalePenalty(depth));
    }

    // Function returns the value of the move from the table
    [[nodiscard]] INLINE int32_t GetBonusMove(const Move mv) const
    {
        return _getPoints(mv.GetStartBoardIndex(), mv.GetTargetField());
    }

    // Resets the content of the table
    void ClearTable();

    // Scales down the values in the table according to the ScaleFactor value
    void ScaleTableDown();

    // Debug function to display statistics about the content
    // Display average points value inside the table and 10 best saved 'moves'
    void DisplayStats() const;

    // ------------------------------
    // Class fields
    // ------------------------------

    // Factor used to scale all values inside the table
    static constexpr int ScaleFactor = 4;

    // Barrier to which all points are capped
    static constexpr int Barrier = 1200;

    private:

    // Functions scales down all points accordingally to currently best hold value if it exceeds the barrier
    [[nodiscard]] int _getPoints(const int figT, const int field) const
    {
        return _table[figT][field];
    }

    // Function used to determine the bonus value of the move, it is here to simplify its manipulation.
    static constexpr int _pointScaleBonus (const int depth)
    {
        return HISTORY_BONUS_COEF * depth + HISTORY_BONUS_BIAS;
    };

    static constexpr int _pointScalePenalty(const int depth)
    {
        return -(HISTORY_PENALTY_COEF * depth / 2+ HISTORY_PENALTY_BIAS);
    }

    int _maxPoints{};
    int _table[Board::BitBoardsCount][Board::BitBoardFields]{};
};

#endif // HISTORICTABLE_H
