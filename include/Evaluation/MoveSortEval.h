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

    int16_t ApplyAttackFieldEffects(int16_t eval, uint64_t blockedFigMap)
    {

    }

    // ------------------------------
    // Class fiels
    // ------------------------------

    static constexpr int16_t ElPassantEval = 100;

    static constexpr int16_t FigureEval[] = {
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

    static constexpr int16_t AttackedFigurePenalty = -100;
    static constexpr int16_t RunAwayPrize = 50;
    static constexpr int16_t KilledFigWhenCheckedPrize = 1000;
};

#endif //MOVESORTEVAL_H
