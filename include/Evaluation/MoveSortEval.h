//
// Created by Jlisowskyy on 3/9/24.
//

#ifndef MOVESORTEVAL_H
#define MOVESORTEVAL_H

#include <cinttypes>

struct MoveSortEval
{

    // ------------------------------
    // Class interaction
    // ------------------------------

    static uint16_t ApplyAttackFieldEffects(uint16_t eval, const  uint64_t pawnAttacks, const uint64_t startField, const uint64_t targetField)
    {
        if ((pawnAttacks & startField) != 0) eval += RunAwayPrize;
        if ((pawnAttacks & targetField) != 0) eval -= AttackedFigurePenalty;

        return eval;
    }

    static uint16_t ApplyPromotionEffects(const uint16_t eval, const size_t nFig)
    {
        return FigureEval[nFig] + eval;
    }

    static uint16_t ApplyKilledFigEffect(const uint16_t eval, const size_t attackFig, const size_t killedFig)
    {
        return eval + FigureEval[killedFig] - FigureEval[attackFig];
    }

    // ------------------------------
    // Class fiels
    // ------------------------------

    // static constexpr uint16_t ElPassantEval = 100;

    static constexpr uint16_t FigureEval[] = {
        100,                // wPawnsIndex,
        300,                // wKnightsIndex,
        300,                // wBishopsIndex,
        500,                // wRooksIndex,
        900,                // wQueensIndex,
          0,                // wKingIndex,
        100,                // bPawnsIndex,
        300,                // bKnightsIndex,
        300,                // bBishopsIndex,
        500,                // bRooksIndex,
        900,                // bQueensIndex,
          0,                // bKingIndex,
    };

    static constexpr uint16_t AttackedFigurePenalty = 50;
    static constexpr uint16_t RunAwayPrize = 50;
    static constexpr uint16_t KilledFigWhenCheckedPrize = 1000;
    static constexpr uint16_t DefaultValue = 1500;
};

#endif //MOVESORTEVAL_H
