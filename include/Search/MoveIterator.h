//
// Created by Jlisowskyy on 7/13/24.
//

#ifndef MOVEITERATOR_H
#define MOVEITERATOR_H
#include "../Board.h"
#include "../MoveGeneration/MoveGenerator.h"


/* Class used to sort moves on the fly, accordingaly to strictly defined rules */
class MoveIterator
{

    enum class MoveSortStages
    {
        TT_MOVE,
        GOOD_PROMOS,
        GOOD_CAPTURES,
        KILLERS,
        GOOD_QUIETS,
        BAD_CAPTURES,
        BAD_QUIETS,

    };

    // ------------------------------
    // Class creation
    // ------------------------------
    public:

    MoveIterator(const Board &board, MoveGenerator &generator, MoveGenerator::payload moves);
    ~MoveIterator() = default;

    MoveIterator() = delete;
    MoveIterator(const MoveIterator&) = delete;
    MoveIterator(MoveIterator&&) = delete;

    MoveIterator& operator=(const MoveIterator&) = delete;
    MoveIterator& operator=(MoveIterator&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    const Board& _board;
    MoveGenerator& _generator;
    MoveGenerator::payload _moves;
};

#endif //MOVEITERATOR_H
