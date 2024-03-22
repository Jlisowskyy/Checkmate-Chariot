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
        static constexpr int Barrier = 2000;

        _table[mv.GetStartBoardIndex()][mv.GetTargetField()] += std::min(depth*depth, Barrier);
    }

    [[nodiscard]] int GetBonusMove(const Move mv) const {
        return _table[mv.GetStartBoardIndex()][mv.GetTargetField()];
    }

    void ClearTable() {
        for (size_t i = 0; i < Board::BoardsCount; ++i)
            std::fill_n(_table[i], Board::BoardFields, 0);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

private:

    int _table[Board::BoardsCount][Board::BoardFields];
};

#endif //HISTORICTABLE_H
