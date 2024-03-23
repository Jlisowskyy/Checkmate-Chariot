//
// Created by Jlisowskyy on 2/29/24.
//

#include "../include/Evaluation/BoardEvaluator.h"

#include "../include/MoveGeneration/BlackPawnMap.h"
#include "../include/MoveGeneration/WhitePawnMap.h"

int32_t BoardEvaluator::PlainMaterialEvaluation(const Board& bd)
{
    int eval = 0;

    for (size_t i = 0; i < Board::BoardsCount; ++i)
        eval += CountOnesInBoard(bd.boards[i]) * BasicFigureValues[i];

    return eval;
}

int32_t BoardEvaluator::NaiveEvaluation2(const Board& bd)
{
    int eval = 0;

    // iterate through boards
    for (size_t bInd = 0; bInd < Board::BoardsCount; ++bInd)
    {
        // extract figures board
        uint64_t figs = bd.boards[bInd];

        // iterate through figures
        while (figs)
        {
            const int figPos = ExtractMsbPos(figs);

            // sum costs offseted by position
            eval += CostsWithPositionsIncluded[bInd][figPos];

            // remove processed figures
            figs ^= maxMsbPossible >> figPos;
        }
    }

    return eval;
}

int32_t BoardEvaluator::NaiveEvaluation3(const Board& bd)
{
    int eval = 0;

    // iterate through boards
    for (size_t bInd = 0; bInd < Board::BoardsCount; ++bInd)
    {
        // extract figures board
        uint64_t figs = bd.boards[bInd];

        // iterate through figures
        while (figs)
        {
            const int figPos = ExtractMsbPos(figs);

            // sum costs offseted by position
            eval += CostsWithPositionsIncluded[bInd][figPos];

            // remove processed figures
            figs ^= maxMsbPossible >> figPos;
        }
    }

    // add bonuses for covering pawns
    eval = _applyBonusForCoveredPawns(bd, eval);

    return eval;
}

int32_t BoardEvaluator::_applyBonusForCoveredPawns(const Board& bd, int32_t eval)
{
    const uint64_t covereWPawns = WhitePawnMap::GetAttackFields(bd.boards[wPawnsIndex]) & bd.boards[wPawnsIndex];
    eval += CountOnesInBoard(covereWPawns) * CoveredWPawnBonus;

    const uint64_t coveredBPawns = BlackPawnMap::GetAttackFields(bd.boards[bPawnsIndex]) & bd.boards[bPawnsIndex];
    eval += CountOnesInBoard(coveredBPawns) * CoveredBPawnBonus;

    return eval;
}

size_t BoardEvaluator::_getMaterialBoardIndex(const Board& bd)
{
    static constexpr int OverflowTables[] = {
        9, 3, 3, 3, 2, 0, 9, 3, 3, 3, 2, 0
    };
    int counts[Board::BoardsCount];
    int overflows = 0;

    for (size_t i = wPawnsIndex; i < wKingIndex; ++i)
    {
        counts[i] = __builtin_popcountll(bd.boards[i]);
        overflows += counts[i] >= OverflowTables[i];
    }

    if (overflows != 0)
        return SIZE_MAX;

    for (size_t i = bPawnsIndex; i < bKingIndex; ++i)
    {
        counts[i] = __builtin_popcountll(bd.boards[i]);
        overflows += counts[i] >= OverflowTables[i];
    }

    if (overflows != 0)
        return SIZE_MAX;

    return _getMaterialBoardIndex(
        counts[bPawnsIndex],
        counts[bKnightsIndex],
        counts[bBishopsIndex],
        counts[bRooksIndex],
        counts[bQueensIndex],
        counts[wPawnsIndex],
        counts[wKnightsIndex],
        counts[wBishopsIndex],
        counts[wRooksIndex],
        counts[wQueensIndex]
    );
}

size_t BoardEvaluator::_getMaterialBoardIndex(const size_t P, const size_t N, const size_t B, const size_t R,
    const size_t Q, const size_t p, const size_t n, const size_t b, const size_t r, const size_t q)
{
    return p * BlackPawnCoef +
           P * WhitePawnCoef +
           n * BlackKnightCoef +
           N * WhiteKnightCoef +
           b * BlackBishopCoef +
           B * WhiteBishopCoef +
           r * BlackRookCoef +
           R * WhiteRookCoef +
           q * BlackQueenCoef +
           Q * WhiteQueenCoef;
}
