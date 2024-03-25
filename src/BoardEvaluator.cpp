//
// Created by Jlisowskyy on 2/29/24.
//

#include "../include/Evaluation/BoardEvaluator.h"

#include "../include/MoveGeneration/BlackPawnMap.h"
#include "../include/MoveGeneration/WhitePawnMap.h"

#include "../include/MoveGeneration/ChessMechanics.h"
#include "../include/MoveGeneration/KnightMap.h"

BoardEvaluator::MaterialArrayT BoardEvaluator::_materialTable =  [] () -> MaterialArrayT
    {
        auto _reverseMaterialIndex = [](const size_t index) -> FigureCountsArrayT
        {
            return {
                (index % BlackPawnCoef) / WhitePawnCoef,
                (index % BlackKnightCoef) / WhiteKnightCoef,
                (index % BlackBishopCoef) / WhiteBishopCoef,
                (index % BlackRookCoef) / WhiteRookCoef,
                (index % BlackQueenCoef) / WhiteQueenCoef,
                index / BlackPawnCoef,
                (index % WhitePawnCoef) / BlackKnightCoef,
                (index % WhiteKnightCoef) / BlackBishopCoef,
                (index % WhiteBishopCoef) / BlackRookCoef,
                (index % WhiteRookCoef) / BlackQueenCoef,
            };
        };

        MaterialArrayT arr{};

        for (size_t i = 0 ; i < MaterialTableSize; ++i)
        {
            int materialValue {};
            auto figArr = _reverseMaterialIndex(i);

            int actPhase{};

            // calculating game phase
            for (size_t j = 0; j < kingIndex; ++j)
                actPhase += static_cast<int>(figArr[j] + figArr[BlackFigStartIndex + j]) * FigurePhases[j];
            actPhase = (actPhase * MaxTapperedCoef + (FullPhase / 2)) / FullPhase; // FullPhase / 2 ?

            // summing up total material values
            for (size_t j = 0; j < kingIndex; ++j)
            {
                const int phasedFigVal = (BasicFigureValues[j]*(MaxTapperedCoef-actPhase) + EndGameFigureValues[j]*actPhase) / MaxTapperedCoef;

                materialValue += static_cast<int>(figArr[j]) * phasedFigVal;
                materialValue -= static_cast<int>(figArr[BlackFigStartIndex + j]) * phasedFigVal;
            }

            // Applying now pawn penalty
            if (figArr[pawnsIndex] == 0)
                materialValue += NoPawnsPenalty;
            if (figArr[BlackFigStartIndex + pawnsIndex] == 0)
                materialValue -= NoPawnsPenalty;

            // Applying Bishop pair bonus
            const size_t totalPawnCount = figArr[pawnsIndex] + figArr[BlackFigStartIndex + pawnsIndex];
            if (figArr[bishopsIndex] == 2)
                materialValue += BishopPairBonus - (static_cast<int>(totalPawnCount)*2 - BishopPairDelta);

            if (figArr[BlackFigStartIndex + bishopsIndex] == 2)
                materialValue -= BishopPairBonus - (static_cast<int>(totalPawnCount)*2 - BishopPairDelta);

            // Applying Knight pair penalty -> Knights are losing value when less pawns are on board
            if (figArr[knightsIndex] == 2)
                materialValue += KnightPairPenalty + (static_cast<int>(totalPawnCount)*2);

            if (figArr[BlackFigStartIndex + knightsIndex] == 2)
                materialValue -= KnightPairPenalty + (static_cast<int>(totalPawnCount)*2);

            // Applying Rook pair penalty
            if (figArr[rooksIndex] == 2)
                materialValue += RookPairPenalty;

            if (figArr[BlackFigStartIndex + rooksIndex] == 2)
                materialValue -= RookPairPenalty;

            arr[i] = static_cast<int16_t>(materialValue);
        }

        // applying draw position scores
        for(const auto& drawPos : MaterialDrawPositionConstelations)
        {
            size_t index{};

            for (size_t i = 0; i < drawPos.size(); ++i)
                index += drawPos[i] * FigCoefs[i];

            arr[index] = 0;
        }

        return arr;
    }();

int32_t BoardEvaluator::DefaultFullEvalFunction(const Board& bd, const int color)
{
    const int whiteEval = Evaluation1(bd);
    return color == WHITE ? whiteEval : -whiteEval;
}

int32_t BoardEvaluator::PlainMaterialEvaluation(const Board& bd)
{
    int32_t eval = 0;

    for (size_t i = 0; i < Board::BoardsCount; ++i)
        eval += CountOnesInBoard(bd.boards[i]) * BasicFigureValues[i];

    return eval;
}

int32_t BoardEvaluator::NaiveEvaluation2(const Board& bd)
{
    int32_t eval = 0;

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
    int32_t eval = 0;

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

int32_t BoardEvaluator::Evaluation1(const Board& bd)
{
    const auto [isSuccess, counts] = _countFigures(bd);
    const int32_t phase = _calcPhase(counts);
    const int32_t materialEval = isSuccess ? _materialTable[_getMaterialBoardIndex(counts)] : _slowMaterialCalculation(counts, phase);

    int32_t positionEval{};
    // summing not tapered fields
    positionEval += _getNotTaperedEval(bd);

    // summing tappered eval
    positionEval += _getTapperedEval(bd, phase);

    return materialEval + positionEval + _applyBonusForCoveredPawns(bd, 0);
}

int32_t BoardEvaluator::Evaluation2(Board& bd)
{
    const auto [isSuccess, counts] = _countFigures(bd);
    const int32_t phase = _calcPhase(counts);

    const int32_t materialEval = isSuccess ? _materialTable[_getMaterialBoardIndex(counts)] : _slowMaterialCalculation(counts, phase);
    const int32_t positionEval = _evaluateFields(bd, phase);
    const int32_t structuralEval = _evaluateStructures(bd, phase);

    return materialEval + positionEval + structuralEval;
}

int32_t BoardEvaluator::_applyBonusForCoveredPawns(const Board& bd, int32_t eval)
{
    const uint64_t covereWPawns = WhitePawnMap::GetAttackFields(bd.boards[wPawnsIndex]) & bd.boards[wPawnsIndex];
    eval += CountOnesInBoard(covereWPawns) * CoveredWPawnBonus;

    const uint64_t coveredBPawns = BlackPawnMap::GetAttackFields(bd.boards[bPawnsIndex]) & bd.boards[bPawnsIndex];
    eval += CountOnesInBoard(coveredBPawns) * CoveredBPawnBonus;

    return eval;
}

std::pair<bool, BoardEvaluator::FigureCountsArrayT> BoardEvaluator::_countFigures(const Board& bd)
{
    static constexpr size_t OverflowTables[] = { 9, 3, 3, 3, 2 };
    FigureCountsArrayT rv{};
    int overflows = 0;

    for (size_t i = pawnsIndex; i < kingIndex; ++i)
    {
        rv[i] = CountOnesInBoard(bd.boards[i]);
        overflows += rv[i] >= OverflowTables[i];
    }

    for (size_t i = pawnsIndex; i < kingIndex; ++i)
    {
        rv[i + 5] = CountOnesInBoard(bd.boards[i + bPawnsIndex]);
        overflows += rv[i + 5] >= OverflowTables[i];
    }

    if (overflows != 0)
        return {false, rv};

    return {true, rv};
}

size_t BoardEvaluator::_getMaterialBoardIndex(const FigureCountsArrayT& counts)
{
    size_t index{};

    for (size_t i = 0; i < counts.size(); ++i)
        index += counts[i] * FigCoefs[i];

    return index;
}

int32_t BoardEvaluator::_slowMaterialCalculation(const FigureCountsArrayT& figArr, const int32_t actPhase)
{
    static constexpr size_t BlackFigStartIndex = 5;

    int32_t materialValue{};

    // summing up total material values
    for (size_t j = 0; j < kingIndex; ++j)
    {
        const int32_t phasedFigVal = (BasicFigureValues[j]*(256-actPhase) + EndGameFigureValues[j]*actPhase) / 256;

        materialValue += static_cast<int32_t>(figArr[j]) * phasedFigVal;
        materialValue -= static_cast<int32_t>(figArr[j + BlackFigStartIndex]) * phasedFigVal;
    }

    // Applying no pawn penalty
    if (figArr[pawnsIndex] == 0)
        materialValue += NoPawnsPenalty;
    if (figArr[BlackFigStartIndex + pawnsIndex] == 0)
        materialValue -= NoPawnsPenalty;

    // Applying Bishop pair bonus
    const size_t totalPawnCount = figArr[pawnsIndex] + figArr[BlackFigStartIndex + pawnsIndex];
    if (figArr[bishopsIndex] == 2)
        materialValue += BishopPairBonus - (static_cast<int32_t>(totalPawnCount)*2 - BishopPairDelta);

    if (figArr[BlackFigStartIndex + bishopsIndex] == 2)
        materialValue -= BishopPairBonus - (static_cast<int32_t>(totalPawnCount)*2 - BishopPairDelta);

    // Applying Knight pair penalty -> Knighs are losing value when less pawns are on board
    if (figArr[knightsIndex] == 2)
        materialValue += KnightPairPenalty + (static_cast<int32_t>(totalPawnCount)*2);

    if (figArr[BlackFigStartIndex + knightsIndex] == 2)
        materialValue -= KnightPairPenalty + (static_cast<int32_t>(totalPawnCount)*2);

    // Applying Rook pair penalty
    if (figArr[rooksIndex] == 2)
        materialValue += RookPairPenalty;

    if (figArr[BlackFigStartIndex + rooksIndex] == 2)
        materialValue -= RookPairPenalty;

    return materialValue;
}

int32_t BoardEvaluator::_calcPhase(const FigureCountsArrayT& figArr)
{
    static constexpr size_t BlackFigStartIndex = 5;
    int32_t actPhase{};

    // calculating game phase
    for (size_t j = 0; j < kingIndex; ++j)
        actPhase += static_cast<int32_t>(figArr[j] + figArr[BlackFigStartIndex + j]) * FigurePhases[j];
    actPhase = (actPhase * MaxTapperedCoef + (FullPhase / 2)) / FullPhase; // FullPhase / 2 ?

    return actPhase;
}

int32_t BoardEvaluator::_getTapperedValue(const int32_t phase, const int32_t min, const int32_t max)
{
    return (min*(MaxTapperedCoef-phase) + max*phase) / MaxTapperedCoef;
}

int32_t BoardEvaluator::_getNotTaperedEval(const Board& bd)
{
    int32_t eval{};
    for (size_t i = knightsIndex ; i <= rooksIndex; ++i)
    {
        const uint64_t wFigs = bd.boards[i];
        const uint64_t bFigs = bd.boards[bPawnsIndex + i];

        eval += _getSimpleFieldEval(
            [&](const int msbInd)
            {
                return BasicBlackPositionValues[i][msbInd];
            }, wFigs
        );

        eval -= _getSimpleFieldEval(
            [=](const int msbInd)
            {
                return BasicBlackPositionValues[i][ConvertToReversedPos(msbInd)];
            }, bFigs
        );
    }

    return eval;
}

int32_t BoardEvaluator::_getTapperedEval(const Board& bd, const int32_t phase)
{
    int32_t openinEval{};

    openinEval += _getSimpleFieldEval(
        [=](const int msbInd)
        {
            return BasicBlackPawnPositionValues[msbInd];
        }, bd.boards[wPawnsIndex]
    );

    openinEval -= _getSimpleFieldEval(
        [=](const int msbInd)
        {
            return BasicBlackPawnPositionValues[ConvertToReversedPos(msbInd)];
        }, bd.boards[bPawnsIndex]
    );

    for (size_t i = queensIndex; i <= kingIndex; ++i)
    {
        const uint64_t wFigs = bd.boards[i];
        const uint64_t bFigs = bd.boards[bPawnsIndex + i];

        openinEval += _getSimpleFieldEval(
            [=](const int msbInd)
            {
                return BasicBlackPositionValues[i][msbInd];
            }, wFigs
        );

        openinEval -= _getSimpleFieldEval(
            [=](const int msbInd)
            {
                return BasicBlackPositionValues[i][ConvertToReversedPos(msbInd)];
            }, bFigs
        );
    }

    int32_t endEval{};

    endEval += _getSimpleFieldEval(
        [=](const int msbInd)
        {
            return BasicBlackPawnPositionEndValues[msbInd];
        }, bd.boards[wPawnsIndex]
    );

    endEval -= _getSimpleFieldEval(
        [=](const int msbInd)
        {
            return BasicBlackPawnPositionEndValues[ConvertToReversedPos(msbInd)];
        }, bd.boards[bPawnsIndex]
    );

    for (size_t i = queensIndex; i <= kingIndex; ++i)
    {
        const uint64_t wFigs = bd.boards[i];
        const uint64_t bFigs = bd.boards[bPawnsIndex + i];

        endEval += _getSimpleFieldEval(
            [=](const int msbInd)
            {
                return BasicBlackPositionEndValues[i][msbInd];
            }, wFigs
        );

        endEval -= _getSimpleFieldEval(
            [=](const int msbInd)
            {
                return BasicBlackPositionEndValues[i][ConvertToReversedPos(msbInd)];
            }, bFigs
        );
    }

    return _getTapperedValue(phase, openinEval, endEval);
}

BoardEvaluator::_fieldEvalInfo_t BoardEvaluator::_evaluatePawns(Board& bd,
    const uint64_t blackPinnedFigs, const uint64_t whitePinnedFigs, const uint64_t fullMap)
{
    _fieldEvalInfo_t rv{};

    const auto [whiteMidEval, whiteEndEval, whiteControlledFields] =
        _processPawnEval<WhitePawnMap, NoOp>(bd, whitePinnedFigs, fullMap);

    const auto [blackMidEval, blackEndEval, blackControlledFields] =
        _processPawnEval<BlackPawnMap, ConvertToReversedPos>(bd, blackPinnedFigs, fullMap);

    rv.midgameEval = whiteMidEval - blackMidEval;
    rv.endgameEval = whiteEndEval - blackEndEval;
    rv.whiteControlledFields = whiteControlledFields;
    rv.blackControlledFields = blackControlledFields;

    return rv;
}

int32_t BoardEvaluator::_evaluateFields(Board& bd, int32_t phase)
{
    static constexpr _fieldEvalInfo_t (*EvalFunctions[])(Board&, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t)
    {
        _processFigEval<KnightMap, _processKnightEval>,
        _processFigEval<BishopMap, _processBishopEval>
    };

    int32_t midEval{};
    int32_t endEval{};
    uint64_t whiteControlledFields{};
    uint64_t blackControlledFields{};
    ChessMechanics mech{bd};

    const uint64_t whiteMap = mech.GetColMap(WHITE);
    const uint64_t blackMap = mech.GetColMap(BLACK);
    const uint64_t fullMap = blackMap | whiteMap;

    const auto [whitePinnedFigs, un1] = mech.GetPinnedFigsMap<ChessMechanics::PinnedFigGen::WoutAllowedTiles>(WHITE, fullMap);
    const auto [blackPinnedFigs, un2] = mech.GetPinnedFigsMap<ChessMechanics::PinnedFigGen::WoutAllowedTiles>(BLACK, fullMap);

    // ------------------------------
    // Evaluating pawn figures
    // ------------------------------

    const auto pEval = _evaluatePawns(bd, blackPinnedFigs, whitePinnedFigs, fullMap);
    const uint64_t whitePawnControl = pEval.whiteControlledFields;
    const uint64_t blackPawnControl = pEval.blackControlledFields;

    midEval += pEval.midgameEval;
    endEval += pEval.endgameEval;

    // ------------------------------
    // Evaluating other pieces
    // ------------------------------

    for(const auto func : EvalFunctions)
    {
        const auto evalAggerg = func(bd, blackPinnedFigs, whitePinnedFigs, whitePawnControl, blackPawnControl, whiteMap, blackMap);

        whiteControlledFields |= evalAggerg.whiteControlledFields;
        blackControlledFields |= evalAggerg.blackControlledFields;

        midEval += evalAggerg.midgameEval;
        endEval += evalAggerg.endgameEval;
    }

    return 0;
}

int32_t BoardEvaluator::_evaluateStructures(const Board& bd, int32_t phase)
{
    return _applyBonusForCoveredPawns(bd, 0);
}
