//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef QUEENMAP_H
#define QUEENMAP_H

#include <cstdint>

#include "BishopMap.h"
#include "RookMap.h"

class QueenMap {
    // ------------------------------
    // Class creation
    // ------------------------------
public:

    constexpr QueenMap() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullMap, const uint64_t allyMap,
                const BishopMap& bMap, const RookMap& rMap)
    {
        const uint64_t moves = bMap.GetMoves(msbInd, fullMap, allyMap) | rMap.GetMoves(msbInd, fullMap, allyMap);

        return moves;
    }

};

#endif //QUEENMAP_H
