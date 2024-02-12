//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "BishopMap.h"
#include "KingMap.h"
#include "KnightMap.h"
#include "PawnMap.h"
#include "QueenMap.h"
#include "RookMap.h"

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

    ChessMechanics() = delete;
    explicit ChessMechanics(Board& bd): board(bd) {}

    ChessMechanics(ChessMechanics& other) = default;
    ChessMechanics& operator=(ChessMechanics&) = delete;
    ~ChessMechanics() = default;

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
        for (size_t i = 0; i < 6; ++i) map |= board.boards[Board::BoardsPerCol*col + i];
        return map;
    }

    // Todo: test later wheter accumulation of blocked maps is faster?
    [[nodiscard]] std::pair<uint64_t, uint8_t> GetBlockedFieldMap(const Color col, const uint64_t fullMap) const {
        uint8_t checksCount{};

        const size_t allyFigInd = col*Board::BoardsPerCol;
        const uint64_t enemyKingMap = board.boards[(1-col)*6 + king];
        const uint8_t enemyKingShift = board.kingPositions[1-col];

        // King attacks generation.
        const uint64_t kingBlockedMap = KingMap::GetMoves(board.kingPositions[col]);

        // Pawns attacks generation.
        const uint64_t pawnsMap = board.boards[col*6 + pawns];
        const uint64_t pawnBlockedMap = col == WHITE
                ? PawnMap::GetWhiteAttackFields(pawnsMap)
                : PawnMap::GetBlackAttackFields(pawnsMap);
        checksCount += (pawnBlockedMap & enemyKingMap) >> enemyKingShift; // += 1 or 0 depending whether hits or not

        // Knight attacks generation.
        const uint64_t knighBlockedMap = _blockIterativeGenerator(board.boards[allyFigInd + knights],
            [&](const int pos) { return KnightMap::GetMoves(pos); }
        );
        checksCount += (knighBlockedMap & enemyKingMap) >> enemyKingShift; // += 1 or 0 depending whether hits or not

        // Rook attacks generation.
        const uint64_t rookBlockedMap = _blockIterativeGenerator(board.boards[allyFigInd + rooks] | board.boards[allyFigInd + queens],
            [&](const int pos) { return RookMap::GetMoves(pos, fullMap); }
        );
        checksCount += (rookBlockedMap & enemyKingMap) >> enemyKingShift; // += 1 or 0 depending whether hits or not

        // Bishop attacks generation.
        const uint64_t bishopBlockedMap = _blockIterativeGenerator(board.boards[allyFigInd + bishops] | board.boards[allyFigInd + queens],
            [&](const int pos) { return BishopMap::GetMoves(pos, fullMap); }
        );
        checksCount += (bishopBlockedMap & enemyKingMap) >> enemyKingShift; // += 1 or 0 depending whether hits or not

        const uint64_t blockedMap = kingBlockedMap | pawnBlockedMap | knighBlockedMap | rookBlockedMap | bishopBlockedMap;
        return { blockedMap, checksCount };
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
