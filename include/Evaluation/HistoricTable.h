//
// Created by Jlisowskyy on 3/22/24.
//

#ifndef HISTORICTABLE_H
#define HISTORICTABLE_H

#include "../MoveGeneration/Move.h"

#include <cstring>

/*
 *  Class used to implement so-called history heuristic. The idea is based on the observation that during the search
 *  we are going to traverse boards that are usually similar to the ones we have already seen. Therefore, we can save
 *  best moves that were played in the past and use them to guide the search in the future.
 *
 *  Resources: https://www.chessprogramming.org/History_Heuristic
 */

// TODO: check whether in bonus there should be a 'depth' or 'ply left'

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

    // Function takes move and depth and increments the move's value in the table
    void SetBonusMove(Move mv, int depth) __attribute__((always_inline))
    {
        _table[mv.GetStartBoardIndex()][mv.GetTargetField()] = static_cast<int16_t>(
            std::min(_pointScale(_table[mv.GetStartBoardIndex()][mv.GetTargetField()], depth), Barrier)
        );
    }

    // Function returns the value of the move from the table
    [[nodiscard]] int32_t GetBonusMove(Move mv) const __attribute__((always_inline))
    {
        return _table[mv.GetStartBoardIndex()][mv.GetTargetField()];
    }

    // Resets the content of the table
    void ClearTable();

    // Scales down the values in the table according to the ScaleFactor value
    void ScaleTableDown();

    // ------------------------------
    // Class fields
    // ------------------------------

    // Factor used to scale all values inside the table
    static constexpr int16_t ScaleFactor = 4;

    // Barrier to which all points are capped
    static constexpr int Barrier = 1200;

    private:
    int16_t _table[Board::BitBoardsCount][Board::BitBoardFields]{};

    // Function used to determine the bonus value of the move, it is here to simplify its manipulation.
    static constexpr auto _pointScale = [](int prevPoints, int depth) constexpr -> int32_t
    {
        return static_cast<int16_t>(prevPoints + depth);
    };
};

#endif // HISTORICTABLE_H
