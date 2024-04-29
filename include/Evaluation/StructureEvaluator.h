//
// Created by Jlisowskyy on 3/26/24.
//

#ifndef STRUCTUREEVALUATOR_H
#define STRUCTUREEVALUATOR_H

#include <cinttypes>

#include "../MoveGeneration/FileMap.h"

/*
 *  Static class used to store various structure evaluation functions.
 */

struct StructureEvaluator
{
    // ------------------------------
    // Class creation
    // ------------------------------

    StructureEvaluator()  = delete;
    ~StructureEvaluator() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // Method used to evaluate whether rook positioned on rooksMsbPos is on open file. Returns bonus for the given rook.
    static int32_t EvalRookOnOpenFile(const Board &bd, int rooksMsbPos, int col);

    // Method simply counts all pawns that are covered by other pawns. Returns the bonus.
    static int32_t EvalPawnChain(uint64_t allyPawns, uint64_t pawnAttacks);

    // Method used to evaluate doubled pawns. Returns penalty for the given pawn. Currently checks only whether there is
    // another pawn on the same file. TODO: reconsider that to something more complicated and precise.
    static int32_t EvalDoubledPawn(uint64_t allyPawns, int pawnsMsbPos, int col);

    // Method used to evaluate isolated pawns. Returns penalty for the given pawn.
    // Currently, it only checks whether there are no own pawns on the neighboring files. TODO: reconsider
    static int32_t EvalIsolatedPawn(uint64_t allyPawns, int pawnsMsbPos);

    // Method used to evaluate passed pawn. Returns bonus for the given pawn when there is no enemy pawn on the same file,
    // and no enemy pawn on the neighboring files.
    static int32_t SimplePassedPawn(uint64_t enemyPawns, int pawnsMsbPos, int col);

    // ------------------------------
    // Class fields
    // ------------------------------

    private:

    // All bonuses for above methods.
    static constexpr int16_t RookSemiOpenFileBonus = 8;
    static constexpr int16_t CoveredPawnBonus      = 4;
    static constexpr int16_t DoubledPawnPenalty    = -25;
    static constexpr int16_t IsolatedPawnPenalty   = -40;
    static constexpr int16_t PassedPawnBonus       = 50;
};

#endif // STRUCTUREEVALUATOR_H
