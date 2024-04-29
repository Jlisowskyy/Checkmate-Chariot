//
// Created by Jlisowskyy on 3/26/24.
//

#ifndef STRUCTUREEVALUATOR_H
#define STRUCTUREEVALUATOR_H

#include <cinttypes>

#include "../MoveGeneration/FileMap.h"

struct  StructureEvaluator
{
    // ------------------------------
    // Class creation
    // ------------------------------

    StructureEvaluator() = delete;
    ~StructureEvaluator() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    static int32_t EvalRookOnOpenFile(const Board& bd, const int msb, const int col)
    {
        const uint64_t allyPawns = bd.BitBoards[col*Board::BitBoardsPerCol + pawnsIndex];
        const uint64_t enemyPawns = bd.BitBoards[SwapColor(col)*Board::BitBoardsPerCol + pawnsIndex];

        int32_t rv{};
        rv += ((FileMap::GetPlainFile(msb) & allyPawns) == 0) * RookSemiOpenFileBonus;
        rv += ((FileMap::GetPlainFile(msb) & (allyPawns | enemyPawns)) == 0) * RookSemiOpenFileBonus;

        return rv;
    }

    static int32_t EvalPawnChain(const uint64_t allyPawns, const uint64_t pawnAttacks)
    {
        return CountOnesInBoard((pawnAttacks & allyPawns))*CoveredPawnBonus;
    }

    static int32_t EvalDoubledPawn(const uint64_t allyPawns, const int msb, const int col)
    {
        return ((allyPawns & FileMap::GetFrontFile(msb, col)) != 0) * DoubledPawnPenalty;
    }

    static int32_t EvalIsolatedPawn(const uint64_t allyPawns, const int msb)
    {
        return ((allyPawns & FileMap::GetNeighborFiles(msb)) == 0) * IsolatedPawnPenalty;
    }

    static int32_t SimplePassedPawn(const uint64_t enemyPawns, const int msb, const int col)
    {
        return ((enemyPawns & FileMap::GetFronFatFile(msb, col)) == 0) * PassedPawnBonus;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    static constexpr int16_t RookSemiOpenFileBonus = 8;
    static constexpr int16_t CoveredPawnBonus = 4;
    static constexpr int16_t DoubledPawnPenalty = -25;
    static constexpr int16_t IsolatedPawnPenalty = -40;
    static constexpr int16_t PassedPawnBonus = 50;
};

#endif //STRUCTUREEVALUATOR_H
