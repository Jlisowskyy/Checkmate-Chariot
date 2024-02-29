//
// Created by Jlisowskyy on 2/28/24.
//

#ifndef BOARDEVALUATOR_H
#define BOARDEVALUATOR_H

#include "../EngineTypeDefs.h"

class BoardEvaluator
{
    // ------------------------------
    // Class creation
    // ------------------------------

    BoardEvaluator() = default;
    ~BoardEvaluator() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------



    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
public:
    static constexpr int32_t QueenValue = 9000;
    static constexpr int32_t RookValue = 5000;
    static constexpr int32_t BishopValue = 3000;
    static constexpr int32_t KnightValue = 3000;
    static constexpr int32_t PawnValue = 1000;

};

#endif //BOARDEVALUATOR_H
