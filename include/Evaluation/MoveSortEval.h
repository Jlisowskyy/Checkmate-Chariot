//
// Created by Jlisowskyy on 3/9/24.
//

#ifndef MOVESORTEVAL_H
#define MOVESORTEVAL_H

#include <cinttypes>

#include "HistoricTable.h"
#include "KillerTable.h"

/*
 * TODO:
 *      - add additional points for moves that gives a check (Will be improved when better check detection is done / make better sorting still),
 * - add points for moves that escapes the danger,
 * - reconsider special sorting for moves when we are in check,
 *      - add SEE for good captures (What about checks?)
 * - allow good quiets to be higher
 * - consider better histories and possibly more types
 * - make check detection during move generation better
 * - improve SEE quality
 *
 * */

/*       TODO: In-Work sorting
 *
 *      The purpose of MoveSortEval is to generate a value which estimates probability of move causing
 *      a beta-cutoff in alpha-beta pruning algorithm. The desired output is to evaluate moves in given order:
 *
 *     1) Previous move that caused beta-cutoff retrieved from TT (Realized inside the search)
 *     2) Capture of the most recently moved figure
 *     3) Queen promos
 *     4) Good captures
 *     5) Good checks
 *     6) Killer Move
 *     7) Counter Move
 *     8) Rest promotions
 *     9) Rest captures
 *     10) All other silent moves according to the history table and pawn control
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
        return PromoEval[nFig] + eval + TacticalBonus;
    }

    // Function applies capture bonus and material balance of the move
    static INLINE int32_t ApplyKilledFigEffect(const int32_t eval, const size_t attackFig, const size_t killedFig, bool& isGoodCapture)
    {
        const int32_t moveEstimation = FigureEval[killedFig] - FigureEval[attackFig];
        isGoodCapture = moveEstimation >= 0;

        return eval + moveEstimation + TacticalBonus;
    }

    // Function applies killer move bonus to the eval
    static INLINE int32_t
    ApplyKillerMoveEffect(const int32_t eval, const KillerTable &kTable, const Move mv, const int ply)
    {
        return eval + KillerMovePrize * kTable.IsKillerMove(mv, ply);
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
    static INLINE int32_t ApplyHistoryTableBonus(const int32_t eval, const Move mv, const HistoricTable &hTable)
    {
        return eval + HTableMultiplier * hTable.GetBonusMove(mv);
    }

    static INLINE int32_t ApplyCheckBonus(const int32_t eval, const bool isChecking)
    {
        return eval + CheckingBonus * isChecking;
    }

    static INLINE int32_t ApplySEEBonus(const int32_t eval, const int32_t SEEBonus)
    {
        return eval + SEEBonus * SEEMultiplier;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    // Ensuring most recent square captures are checked first
    static constexpr int16_t MostRecentSquarePrize = 8000;
    // Ensuring queen promo is checked as best promotion
    static constexpr int16_t QueenPromoPrize = 6000;
    // Bonus for tactical move, applied for captures and promos - important to ensure some rules
    static constexpr int16_t TacticalBonus = 1500;

    // Both values ensure that given move types are checked accoringly to the rules
    // Note: we are still above first bad captures ~ 1300
    static constexpr int16_t KillerMovePrize       = 1425;
    static constexpr int16_t CounterMovePrize      = 1350;

    static constexpr int16_t CheckingBonus = 1000;

    // small prizes to distinguish nearly identical moves
    static constexpr int16_t AttackedFigurePenalty = -50;
    static constexpr int16_t RunAwayPrize          = 50;

    static constexpr int16_t SEEMultiplier = 3;
    static constexpr int16_t HTableMultiplier = 1;

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

    // Note:
    //      - bigger quen prize to ensure is check as on of the first moves
    //      - neutral prizes with offset for other figures to ensures good captures are always checked first
    static constexpr int16_t PromoEval[] = {
        100, // wPawnsIndex,
        1 - 150, // wKnightsIndex,
        2 - 150, // wBishopsIndex,
        3 - 150, // wRooksIndex,
        QueenPromoPrize, // wQueensIndex,
        0,   // wKingIndex,
        100, // bPawnsIndex,
        1 - 150, // bKnightsIndex,
        2 - 150, // bBishopsIndex,
        3 - 150, // bRooksIndex,
        QueenPromoPrize, // bQueensIndex,
        0,   // bKingIndex,
    };
};

#endif // MOVESORTEVAL_H
