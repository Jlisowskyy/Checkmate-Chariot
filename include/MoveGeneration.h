//
// Created by Jlisowskyy on 12/30/23.
//

#ifndef MOVEGENERATION_H
#define MOVEGENERATION_H

#include <array>

#include "BitOperations.h"
#include "EngineTypeDefs.h"

[[nodiscard]] constexpr std::array<uint64_t, Board::BoardFields> GenStaticMoves(const int maxMovesCount,
    const int *movesCords, const int *rowCords)
{
    std::array<uint64_t, Board::BoardFields> movesRet{};

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            const int bInd = 8*y + x;
            const int msbInd = ConvertToReversedPos(bInd);

            uint64_t packedMoves = 0;
            for (size_t i = 0; i < maxMovesCount; ++i)  {
                const int moveYCord = (bInd + movesCords[i]) / 8;
                const int knightYCord = bInd / 8;

                if (const int moveInd = bInd + movesCords[i];
                    moveInd >= 0 && moveInd < 64 && moveYCord == knightYCord + rowCords[i])
                    packedMoves |= 1LLU << moveInd;
            }

            movesRet[msbInd] = packedMoves;
        }
    }

    return movesRet;
}

#endif //MOVEGENERATION_H
