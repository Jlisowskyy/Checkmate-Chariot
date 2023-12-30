//
// Created by Jlisowskyy on 12/30/23.
//

#include "../include/KingMap.h"
#include "../include/BitOperations.h"

constexpr uint64_t KingMap::GetMoves(const int msbInd, const uint64_t, const uint64_t allyMap) const {
    const uint64_t moves = movesMap[msbInd];

    return ClearAFromIntersectingBits(moves, allyMap);
}

constexpr std::array<uint64_t, Board::BoardFields> KingMap::_genMoves() {
    std::array<uint64_t, Board::BoardFields> movesRet{};

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            const int bInd = 8 * y + x;
            const int msbInd = ConvertToReversedPos(bInd);

            uint64_t moves = 0;
            for (size_t i = 0; i < 8; ++i) {
                const int moveYCord = (bInd + moveCords[i]) / 8;
                const int kingYCord = bInd / 8;

                if (bInd + moveCords[i] >= Board::BoardFields || bInd + moveCords[i] < 0
                    || kingYCord + rowCords[i] != moveYCord) continue;

                const uint64_t move = 1LLU << (bInd + moveCords[i]);
                moves |= move;
            }

            movesRet[msbInd] = moves;
        }
    }

    return movesRet;
}