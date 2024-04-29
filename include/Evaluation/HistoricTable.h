//
// Created by Jlisowskyy on 3/22/24.
//

#ifndef HISTORICTABLE_H
#define HISTORICTABLE_H

#include "../MoveGeneration/Move.h"

#include <cstring>

struct HistoricTable {
    // ------------------------------
    // Class creation
    // ------------------------------

    HistoricTable() = default;
    ~HistoricTable() = default;

    HistoricTable(HistoricTable&&) = delete;
    HistoricTable(const HistoricTable&) = delete;

    HistoricTable& operator=(const HistoricTable&) = delete;
    HistoricTable& operator=(HistoricTable&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void SetBonusMove(const Move mv, const int depth) {
        static constexpr int Barrier = 2400;

        _table[mv.GetStartBoardIndex()][mv.GetTargetField()] = std::min(_table[mv.GetStartBoardIndex()][mv.GetTargetField()] + depth, Barrier);
    }

    [[nodiscard]] int GetBonusMove(const Move mv) const {
        return _table[mv.GetStartBoardIndex()][mv.GetTargetField()];
    }

    void ClearTable() {
        for (size_t i = 0; i < Board::BitBoardsCount; ++i)
            std::fill_n(_table[i], Board::BitBoardFields, 0);
    }

    void ScaleTableDown() {
        for (auto & figureMap : _table)
            for (short & field : figureMap)
                field /= ScaleFactor;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int16_t ScaleFactor = 4;

private:

    int16_t _table[Board::BitBoardsCount][Board::BitBoardFields];
};

#endif //HISTORICTABLE_H
