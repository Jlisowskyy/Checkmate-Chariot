//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef KNIGHTMAP_H
#define KNIGHTMAP_H

#include <array>

#include "EngineTypeDefs.h"
#include "BitOperations.h"

class KnightMap {
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    constexpr KnightMap();

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] uint64_t GetMoves(int msbInd, uint64_t, uint64_t allyMap) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    std::array<uint64_t, Board::BoardFields> movesMap{};

    static constexpr size_t maxMovesCount = 8;
    static constexpr int moves[] = { 6, 15, 17, 10, -6, -15, -17, -10 };
};

#endif //KNIGHTMAP_H
