//
// Created by Jlisowskyy on 12/28/23.
//

#include "../include/KnightMap.h"

constexpr KnightMap::KnightMap() {
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

uint64_t KnightMap::GetMoves(const int msbInd, const uint64_t, const uint64_t allyMap) const {
    const uint64_t moves = movesMap[msbInd];

    return ClearAFromIntersectingBits(moves, allyMap);
}
