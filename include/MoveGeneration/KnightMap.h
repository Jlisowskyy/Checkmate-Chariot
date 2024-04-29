//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef KNIGHTMAP_H
#define KNIGHTMAP_H

#include <array>

#include "MoveGenerationUtils.h"

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

    [[nodiscard]] static constexpr size_t GetBoardIndex(int color);

    [[nodiscard]] static constexpr uint64_t
    GetMoves(int msbInd, [[maybe_unused]] uint64_t = 0, [[maybe_unused]] uint64_t = 0);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr size_t maxMovesCount = 8;

    // Describes knight possible moves coordinates.
    static constexpr int movesCords[] = {6, 15, 17, 10, -6, -15, -17, -10};

    // Accordingly describes y positions after the move relatively to knight's y position.
    // Used to omit errors during generation.
    static constexpr int rowCords[] = {1, 2, 2, 1, -1, -2, -2, -1};

    static constexpr std::array<uint64_t, Board::BitBoardFields> movesMap =
        GenStaticMoves(maxMovesCount, movesCords, rowCords);
};

constexpr size_t KnightMap::GetBoardIndex(const int color) { return Board::BitBoardsPerCol * color + knightsIndex; }

constexpr uint64_t KnightMap::GetMoves(const int msbInd, const uint64_t, const uint64_t) { return movesMap[msbInd]; }

#endif // KNIGHTMAP_H
