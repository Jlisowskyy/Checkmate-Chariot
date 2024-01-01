//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include "BitOperations.h"
#include "EngineTypeDefs.h"
#include "MoveGeneration/BishopMap.h"
#include "MoveGeneration/KingMap.h"
#include "MoveGeneration/KnightMap.h"
#include "MoveGeneration/PawnMap.h"
#include "MoveGeneration/QueenMap.h"
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

    [[nodiscard]] uint64_t GetBlockedFieldMap(const Color col, const uint64_t enemyMap, const uint64_t fullMap) const {
        uint64_t blockedMap = 0;

        // Pawns attacks generation.
        const uint64_t pawnsMap = board.boards[col*6 + pawns];

        blockedMap |= col == WHITE
                ? PawnMap::GetWhiteAttacks(pawnsMap, enemyMap)
                : PawnMap::GetBlackAttacks(pawnsMap, enemyMap);

        // King attacks generation.
        blockedMap |= KingMap::GetMoves(ExtractMsbPos(board.boards[col*6 + king]));

        // Knight attacks generation.
        blockedMap |= _blockIterativeGenerator(board.boards[col*6 + knights],
            [&](const int pos) { return KnightMap::GetMoves(pos); }
        );

        // Rook attacks generation.
        blockedMap |= _blockIterativeGenerator(board.boards[col*6 + rooks],
            [&](const int pos) { return RookMap::GetMoves(pos, fullMap); }
        );

        // Bishop attacks generation.
        blockedMap |= _blockIterativeGenerator(board.boards[col*6 + bishops],
            [&](const int pos) { return BishopMap::GetMoves(pos, fullMap); }
        );

        // Queen attacks generation.
        blockedMap |= _blockIterativeGenerator(board.boards[col*6 + queens],
            [&](const int pos) { return QueenMap::GetMoves(pos, fullMap); }
        );

        return blockedMap;
    }

    [[nodiscard]] uint64_t GetBlockedFigsMap(const Color col, const uint64_t enemyMap, const uint64_t allyMap, const uint64_t fullMap) const {

        return 0;
    }
    // ------------------------------
    // private methods
    // ------------------------------
private:

    template<class moveGenerator>
    [[nodiscard]] static uint64_t _blockIterativeGenerator(uint64_t board, moveGenerator mGen) {
        uint64_t blockedMap = 0;

        while (board != 0) {
            const int figPos = ExtractMsbPos(board);
            board ^= (minMsbPossible << figPos);

            blockedMap |= mGen(figPos);
        }

        return blockedMap;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    Board& board;
};



#endif //CHESSMECHANICS_H
