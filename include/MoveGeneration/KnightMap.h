//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef KNIGHTMAP_H
#define KNIGHTMAP_H

#include <array>

#include "../EngineTypeDefs.h"
#include "MoveGeneration.h"

class KnightMap
{
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------
   public:
    KnightMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr size_t GetBoardIndex(const int color)
    {
        return Board::BoardsPerCol * color + knightsIndex;
    }

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd, [[maybe_unused]] const uint64_t _ = 0,
                                                     [[maybe_unused]] const uint64_t __ = 0)
    {
        return movesMap[msbInd];
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr size_t maxMovesCount = 8;

    // Describes knight possible moves cordinates.
    static constexpr int movesCords[] = {6, 15, 17, 10, -6, -15, -17, -10};

    // Describes accordingly y positions after the move relatively to knight's y position.
    // Used to omit errors during generation.
    static constexpr int rowCords[] = {1, 2, 2, 1, -1, -2, -2, -1};

    static constexpr std::array<uint64_t, Board::BoardFields> movesMap =
        GenStaticMoves(maxMovesCount, movesCords, rowCords);
};

#endif  // KNIGHTMAP_H
