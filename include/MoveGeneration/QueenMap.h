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

    [[nodiscard]] static constexpr size_t GetBoardIndex(const int color)
    {
        return Board::BoardsPerCol * color + queensIndex;
    }

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullMap,
                                                     [[maybe_unused]] const uint64_t _ = 0)
    {
        return BishopMap::GetMoves(msbInd, fullMap) | RookMap::GetMoves(msbInd, fullMap);
    }
};

#endif //QUEENMAP_H
