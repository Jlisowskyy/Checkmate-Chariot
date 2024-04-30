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
        const int32_t eval, const uint64_t pawnAttacks, const uint64_t startField, const uint64_t targetField
    ) __attribute__((always_inline))
    {
        return eval
               + ((pawnAttacks & startField) != 0) * RunAwayPrize
               + ((pawnAttacks & targetField) != 0) * AttackedFigurePenalty;
    }

    static int32_t ApplyPromotionEffects(const int32_t eval, const size_t nFig) __attribute__((always_inline))
    {
        return FigureEval[nFig] + eval + PromotionBonus;
    }

    static int32_t ApplyKilledFigEffect(const int32_t eval, const size_t attackFig, const size_t killedFig) __attribute__((always_inline))
    {
        return eval + FigureEval[killedFig] - FigureEval[attackFig] + CaptureBonus;
    }

    static int32_t
    ApplyKillerMoveEffect(const int32_t eval, const KillerTable &kTable, const Move mv, const int depthLeft) __attribute__((always_inline))
    {
        return eval + KillerMovePrize * kTable.IsKillerMove(mv, depthLeft);
    }

    static int32_t ApplyCounterMoveEffect(const int32_t eval, const PackedMove counterMove, const Move move) __attribute__((always_inline))
    {
        return eval + CounterMovePrize * (move.GetPackedMove() == counterMove);
    }

    static int32_t
    ApplyCaptureMostRecentSquareEffect(const int32_t eval, const int mostRecentSquareMsb, const int moveSquare) __attribute__((always_inline))
    {
        return eval + MostRecentSquarePrize * (mostRecentSquareMsb == moveSquare);
    }

    static int32_t ApplyHistoryTableBonus(const int32_t eval, const Move mv, const HistoricTable &hTable) __attribute__((always_inline))
    {
        return eval + hTable.GetBonusMove(mv);
    }

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
