//
// Created by Jlisowskyy on 4/29/24.
//

#include "../include/Evaluation/StructureEvaluator.h"

int32_t StructureEvaluator::EvalRookOnOpenFile(const Board &bd, const int rooksMsbPos, const int col)
{
    const uint64_t allyPawns  = bd.BitBoards[col * Board::BitBoardsPerCol + pawnsIndex];
    const uint64_t enemyPawns = bd.BitBoards[SwapColor(col) * Board::BitBoardsPerCol + pawnsIndex];

    int32_t rv{};

    // We give half of the bonus for each missing color's pawns on the file. That together gives full bonus: 16.
    rv += ((FileMap::GetPlainFile(rooksMsbPos) & allyPawns) == 0) * RookSemiOpenFileBonus;
    rv += ((FileMap::GetPlainFile(rooksMsbPos) & (allyPawns | enemyPawns)) == 0) * RookSemiOpenFileBonus;

    return rv;
}
int32_t StructureEvaluator::EvalPawnChain(const uint64_t allyPawns, const uint64_t pawnAttacks)
{
    return CountOnesInBoard((pawnAttacks & allyPawns)) * CoveredPawnBonus;
}
int32_t StructureEvaluator::EvalDoubledPawn(const uint64_t allyPawns, const int pawnsMsbPos, const int col)
{
    return ((allyPawns & FileMap::GetFrontFile(pawnsMsbPos, col)) != 0) * DoubledPawnPenalty;
}
int32_t StructureEvaluator::EvalIsolatedPawn(const uint64_t allyPawns, const int pawnsMsbPos)
{
    return ((allyPawns & FileMap::GetNeighborFiles(pawnsMsbPos)) == 0) * IsolatedPawnPenalty;
}
int32_t StructureEvaluator::SimplePassedPawn(const uint64_t enemyPawns, const int pawnsMsbPos, const int col)
{
    return ((enemyPawns & FileMap::GetFronFatFile(pawnsMsbPos, col)) == 0) * PassedPawnBonus;
}
