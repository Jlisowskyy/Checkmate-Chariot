//
// Created by Jlisowskyy on 3/9/24.
//

#ifndef MOVESORTEVAL_H
#define MOVESORTEVAL_H

#include <cinttypes>

#include "KillerTable.h"

/*              LIST OF SORTING RULES:
 *  1) Avoid fields that are attacked by enemy pawn
 *  2) Focus moves that are realeasing enemy pawn pressure on them
 *  3) Focus profitable attacks, avoid non profitable
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

    static int16_t ApplyAttackFieldEffects(int16_t eval, const uint64_t pawnAttacks, const uint64_t startField,
                                            const uint64_t targetField)
    {
        if ((pawnAttacks & startField) != 0)
            eval += RunAwayPrize;
        if ((pawnAttacks & targetField) != 0)
            eval += AttackedFigurePenalty;

        return eval;
    }

    static int16_t ApplyPromotionEffects(const int16_t eval, const size_t nFig) { return FigureEval[nFig] + eval; }

    static int16_t ApplyKilledFigEffect(const int16_t eval, const size_t attackFig, const size_t killedFig)
    {
        return eval + FigureEval[killedFig] - FigureEval[attackFig];
    }

    static int16_t ApplyKillerMoveEffect(const int16_t eval, const KillerTable& kTable, const Move mv, const int depthLeft)
    {
        return eval + KillerMovePrize * kTable.IsKillerMove(mv, depthLeft);
    }

    // ------------------------------
    // Class fiels
    // ------------------------------

    static constexpr int16_t FigureEval[] = {
        100,  // wPawnsIndex,
        300,  // wKnightsIndex,
        300,  // wBishopsIndex,
        500,  // wRooksIndex,
        900,  // wQueensIndex,
        0,    // wKingIndex,
        100,  // bPawnsIndex,
        300,  // bKnightsIndex,
        300,  // bBishopsIndex,
        500,  // bRooksIndex,
        900,  // bQueensIndex,
        0,    // bKingIndex,
    };

    static constexpr int16_t AttackedFigurePenalty = -50;
    static constexpr int16_t RunAwayPrize = 50;
    static constexpr int16_t KillerMovePrize = 150;
};

#endif  // MOVESORTEVAL_H
