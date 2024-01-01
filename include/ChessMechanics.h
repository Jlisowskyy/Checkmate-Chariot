//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include "BitOperations.h"
#include "EngineTypeDefs.h"
#include "MoveGeneration/KingMap.h"
#include "MoveGeneration/KnightMap.h"
#include "MoveGeneration/PawnMap.h"
#include "MoveGeneration/RookMap.h"

/*              General optimizations TODO:
 *      - test all msb and lsb extractions
 *      - add and test AVX support
 *      - test other ideas for hashing
 *
 */

class ChessMechanics {
    // ------------------------------
    // Class Creation
    // ------------------------------
public:

    explicit ChessMechanics(Board& bd): board(bd) {}

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] uint64_t GetFullMap() const{
        uint64_t map = 0;
        for(const auto m : board.boards) map |= m;
        return map;
    }

    [[nodiscard]] uint64_t GetColMap(const Color col) const {
        uint64_t map = 0;
        for (size_t i = 0; i < 6; ++i) map |= board.boards[6*col + i];
        return map;
    }

    [[nodiscard]] uint64_t GetBlockedMap(const Color col, const uint64_t enemyMap, const uint64_t allyMap, const uint64_t fullMap) {
        uint64_t blockedMap = 0;

        // Pawns attacks generation.
        const uint64_t pawnsMap = board.boards[col*6 + pawns];

        blockedMap |= col == WHITE
                ? PawnMap::GetWhiteAttacks(pawnsMap, enemyMap)
                : PawnMap::GetBlackAttacks(pawnsMap, enemyMap);

        // Knight attacks generation.
        uint64_t knightMap = board.boards[col*6 + knights];
        while (knightMap != 0) {
            const int knightPos = ExtractMsbPos(knightMap);
            knightMap ^= (minMsbPossible << knightPos);

            blockedMap |= KnightMap::GetMoves(knightPos);
        }

        // King attacks generation.
        blockedMap |= KingMap::GetMoves(ExtractMsbPos(board.boards[col*6 + king]));

        uint64_t rookMap = board.boards[col*6 + rooks];
        while (rookMap != 0) {
            const int rookPos = ExtractMsbPos(rookMap);
            rookMap ^= (minMsbPossible << rookPos);

            // blockedMap |= RookMap::GetMoves(rookPos, fullMap);
        }

        return blockedMap;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    Board& board;
};



#endif //CHESSMECHANICS_H
