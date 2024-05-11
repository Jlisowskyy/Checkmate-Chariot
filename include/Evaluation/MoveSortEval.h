//
// Created by Jlisowskyy on 3/9/24.
//

#ifndef MOVESORTEVAL_H
#define MOVESORTEVAL_H

#include <cinttypes>

#include "HistoricTable.h"
#include "KillerTable.h"

/*
 *      The purpose of MoveSortEval is to generate a value which estimates probability of move causing
 *      a beta-cutoff in alpha-beta pruning algorithm. The desired output is to evaluate moves in given order:
 *
 *     1) Previous move that caused beta-cutoff retrieved from TT (Realized inside the search
 *     2) Capture of the most recently moved figure
 *     3) All promotions
 *     4) All captures sorted in order from best to worst
 *     5) All Killer moves
 *     6) All Counter Moves
 *     7) All other silent moves according to the history table and pawn control
 *
 * */

struct MoveSortEval
{
    // ------------------------------
    // Class interaction
    // ------------------------------

    // Function applies pawns controlling penalty to the eval
    static INLINE int32_t ApplyAttackFieldEffects(
        const int32_t eval, const uint64_t pawnAttacks, const uint64_t startField, const uint64_t targetField
    )
    {
        return eval + ((pawnAttacks & startField) != 0) * RunAwayPrize +
               ((pawnAttacks & targetField) != 0) * AttackedFigurePenalty;
    }

    // Function applies promotion bonus to the eval
    static INLINE int32_t ApplyPromotionEffects(const int32_t eval, const size_t nFig)
    {
        return FigureEval[nFig] + eval + PromotionBonus;
    }

    // Function applies capture bonus and material balance of the move
    static INLINE int32_t ApplyKilledFigEffect(const int32_t eval, const size_t attackFig, const size_t killedFig)

    {
        return eval + FigureEval[killedFig] - FigureEval[attackFig] + CaptureBonus;
    }

    // Function applies killer move bonus to the eval
    static INLINE int32_t
    ApplyKillerMoveEffect(const int32_t eval, const KillerTable &kTable, const Move mv, const int depthLeft)

    {
        return eval + KillerMovePrize * kTable.IsKillerMove(mv, depthLeft);
    }

    // Function applies Counter Move bonus to the eval
    static INLINE int32_t ApplyCounterMoveEffect(const int32_t eval, const PackedMove counterMove, const Move move)

    {
        return eval + CounterMovePrize * (move.GetPackedMove() == counterMove);
    }

    // Function applies bonus to the eval if move is a capture of the most recently moved figure
    static INLINE int32_t
    ApplyCaptureMostRecentSquareEffect(const int32_t eval, const int mostRecentSquareMsb, const int moveSquare)

    {
        return eval + MostRecentSquarePrize * (mostRecentSquareMsb == moveSquare);
    }

    // Function applies bonus according to the history table
    static INLINE int32_t ApplyHistoryTableBonus(const int32_t eval, Move mv, const HistoricTable &hTable)

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
    static constexpr int16_t KillerMovePrize       = 1500;
    static constexpr int16_t CounterMovePrize      = 1300;
    static constexpr int16_t MostRecentSquarePrize = 1600;
    static constexpr int16_t CaptureBonus          = 2500;
    static constexpr int16_t PromotionBonus        = 4000;
};

#endif // MOVESORTEVAL_H
