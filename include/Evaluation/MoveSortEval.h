//
// Created by Jlisowskyy on 3/9/24.
//

#ifndef MOVESORTEVAL_H
#define MOVESORTEVAL_H

#include <cinttypes>

#include "HistoricTable.h"
#include "KillerTable.h"

/*              LIST OF SORTING RULES:
 *  1) Avoid fields that are attacked by enemy pawn
 *  2) Focus moves that are releasing enemy pawn pressure on them
 *  3) Focus profitable attacks, avoid non-profitable
 *  4) Firstly follow promotion paths
 *
 *  TODOS:
 *  1) Focus attacks on pinned figs
 */

struct MoveSortEval
{
    // ------------------------------
    // Class interaction
    // ------------------------------

    static int32_t ApplyAttackFieldEffects(
        int32_t eval, uint64_t pawnAttacks, uint64_t startField, uint64_t targetField
    );

    static int32_t ApplyPromotionEffects(int32_t eval, size_t nFig);

    static int32_t ApplyKilledFigEffect(int32_t eval, size_t attackFig, size_t killedFig);

    static int32_t
    ApplyKillerMoveEffect(int32_t eval, const KillerTable &kTable, Move mv, int depthLeft);

    static int32_t ApplyCounterMoveEffect(int32_t eval, PackedMove counterMove, Move move);

    static int32_t
    ApplyCaptureMostRecentSquareEffect(int32_t eval, int mostRecentSquareMsb, int moveSquare);

    static int32_t ApplyHistoryTableBonus(int32_t eval, Move mv, const HistoricTable &hTable);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int16_t FigureEval[] = {
        100, // wPawnsIndex,
        300, // wKnightsIndex,
        300, // wBishopsIndex,
        500, // wRooksIndex,
        900, // wQueensIndex,
        0,   // wKingIndex,
        100, // bPawnsIndex,
        300, // bKnightsIndex,
        300, // bBishopsIndex,
        500, // bRooksIndex,
        900, // bQueensIndex,
        0,   // bKingIndex,
    };

    static constexpr int16_t AttackedFigurePenalty = -50;
    static constexpr int16_t RunAwayPrize          = 50;
    static constexpr int16_t KillerMovePrize       = 150;
    static constexpr int16_t CounterMovePrize      = 200;
    static constexpr int16_t MostRecentSquarePrize = 1600;
    static constexpr int16_t CaptureBonus          = 2500;
    static constexpr int16_t PromotionBonus        = 4000;
};

#endif // MOVESORTEVAL_H
