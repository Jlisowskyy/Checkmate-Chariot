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
    template <EvalMode mode = EvalMode::BaseMode>
    static INLINE int32_t EvalRookOnOpenFile(const Board &bd, const int rooksMsbPos, const int col)

    {
        const uint64_t allyPawns  = bd.BitBoards[col * Board::BitBoardsPerCol + pawnsIndex];
        const uint64_t enemyPawns = bd.BitBoards[SwapColor(col) * Board::BitBoardsPerCol + pawnsIndex];

        int32_t rv{};

        // We give half of the bonus for each missing color's pawns on the file. That together gives full bonus: 16.
        rv += ((FileMap::GetPlainFile(rooksMsbPos) & allyPawns) == 0) * RookSemiOpenFileBonus;
        rv += ((FileMap::GetPlainFile(rooksMsbPos) & (allyPawns | enemyPawns)) == 0) * RookSemiOpenFileBonus;

        return rv;
    }

    // Method simply counts all pawns that are covered by other pawns. Returns the bonus.
    template <EvalMode mode = EvalMode::BaseMode>
    static INLINE int32_t EvalPawnChain(const uint64_t allyPawns, const uint64_t pawnAttacks)
    {
        return CountOnesInBoard((pawnAttacks & allyPawns)) * CoveredPawnBonus;
    }

    // Method used to evaluate doubled pawns. Returns penalty for the given pawn. Currently checks only whether there is
    // another pawn on the same file. TODO: reconsider that to something more complicated and precise.
    template <EvalMode mode = EvalMode::BaseMode>
    static INLINE int32_t EvalDoubledPawn(const uint64_t allyPawns, const int pawnsMsbPos, const int col)
    {
        return ((allyPawns & FileMap::GetFrontFile(pawnsMsbPos, col)) != 0) * DoubledPawnPenalty;
    }

    // Method used to evaluate isolated pawns. Returns penalty for the given pawn.
    // Currently, it only checks whether there are no own pawns on the neighboring files. TODO: reconsider
    template <EvalMode mode = EvalMode::BaseMode>
    static INLINE int32_t EvalIsolatedPawn(const uint64_t allyPawns, const int pawnsMsbPos)
    {
        return ((allyPawns & FileMap::GetNeighborFiles(pawnsMsbPos)) == 0) * IsolatedPawnPenalty;
    }

    // Method used to evaluate passed pawn. Returns bonus for the given pawn when there is no enemy pawn on the same
    // file, and no enemy pawn on the neighboring files.
    template <EvalMode mode = EvalMode::BaseMode>
    static INLINE int32_t SimplePassedPawn(const uint64_t enemyPawns, const int pawnsMsbPos, const int col)
    {
        return ((enemyPawns & FileMap::GetFronFatFile(pawnsMsbPos, col)) == 0) * PassedPawnBonus;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    // All bonuses for above methods.
    static constexpr int16_t RookSemiOpenFileBonus = 8;
    static constexpr int16_t CoveredPawnBonus      = 4;
    static constexpr int16_t DoubledPawnPenalty    = -20;
    static constexpr int16_t IsolatedPawnPenalty   = -20;
    static constexpr int16_t PassedPawnBonus       = 30;
};

#endif // STRUCTUREEVALUATOR_H
