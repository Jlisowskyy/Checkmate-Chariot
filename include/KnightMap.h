//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef KNIGHTMAP_H
#define KNIGHTMAP_H

#include <array>

#include "EngineTypeDefs.h"

class KnightMap {
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    constexpr KnightMap() {
        movesMap = std::array<uint64_t, Board::BoardFields>{};
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                const int mapInd = 63 - (y*8 + x);
                uint64_t packedMoves = 0;

                for (const int move : moves) {
                    if (const int moveInd = mapInd + move; moveInd >= 0 && moveInd < 64)
                        packedMoves |= 1LLU << moveInd;
                }

                movesMap[mapInd] = packedMoves;
            }
        }
    }

    // ------------------------------
    // private methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    constexpr std::array<uint64_t, Board::BoardFields> movesMap;

    static constexpr size_t maxMovesCount = 8;
    static constexpr int moves[] = { 6, 15, 17, 10, -6, -15, -17, -10 };
};



#endif //KNIGHTMAP_H
