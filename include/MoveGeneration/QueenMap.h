//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef QUEENMAP_H
#define QUEENMAP_H

#include <cstdint>

#include "BishopMap.h"
#include "RookMap.h"

class QueenMap
{
    // ------------------------------
    // Class creation
    // ------------------------------
   public:
    constexpr QueenMap() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr size_t GetBoardIndex(int color);

    [[nodiscard]] static constexpr uint64_t GetMoves(int msbInd, uint64_t fullMap, [[maybe_unused]] uint64_t = 0);
};

constexpr size_t QueenMap::GetBoardIndex(const int color)
{
    return Board::BitBoardsPerCol * color + queensIndex;
}

constexpr uint64_t QueenMap::GetMoves(const int msbInd, const uint64_t fullMap, const uint64_t)
{
    return BishopMap::GetMoves(msbInd, fullMap) | RookMap::GetMoves(msbInd, fullMap);
}

#endif  // QUEENMAP_H
