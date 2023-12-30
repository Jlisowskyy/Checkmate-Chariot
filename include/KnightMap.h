//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef KNIGHTMAP_H
#define KNIGHTMAP_H

#include <array>

#include "EngineTypeDefs.h"
#include "BitOperations.h"
#include "MoveGeneration.h"

class KnightMap {
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------
public:
    constexpr KnightMap(){
        movesMap = GenStaticMoves(maxMovesCount, movesCords, rowCords);
    }

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] uint64_t GetMoves(const int msbInd, const uint64_t, const uint64_t allyMap) const{
        const uint64_t moves = movesMap[msbInd];

        return ClearAFromIntersectingBits(moves, allyMap);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    std::array<uint64_t, Board::BoardFields> movesMap{};

    static constexpr size_t maxMovesCount = 8;

    // Describes knight possible moves cordinates.
    static constexpr int movesCords[] = { 6, 15, 17, 10, -6, -15, -17, -10 };

    // Describes accordingly y positions after the move relatively to knight's y position.
    // Used to omit errors during generation.
    static constexpr int rowCords[] = { 1, 2, 2, 1, -1, -2, -2, -1 };
};

#endif //KNIGHTMAP_H
