//
// Created by Jlisowskyy on 2/29/24.
//

#include "../include/Evaluation/BoardEvaluator.h"

#include "../include/MoveGeneration/BlackPawnMap.h"
#include "../include/MoveGeneration/WhitePawnMap.h"

#include "../include/MoveGeneration/ChessMechanics.h"
#include "../include/MoveGeneration/KnightMap.h"
#include "../include/MoveGeneration/QueenMap.h"

BoardEvaluator::MaterialArrayT BoardEvaluator::_materialTable =  [] () -> MaterialArrayT
    {
        // Lambda used to based on given index return array of figure counts
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

        // processing all position with standard procedure
        for (size_t i = 0 ; i < MaterialTableSize; ++i)
        {
            auto figArr = _reverseMaterialIndex(i);

            int32_t phase = BoardEvaluator::_calcPhase(figArr);
            int32_t materialValue = BoardEvaluator::_slowMaterialCalculation(figArr, phase);

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

int32_t BoardEvaluator::DefaultFullEvalFunction(Board& bd, const int color)
{
    const int whiteEval = Evaluation2(bd);
    return (color == WHITE ? whiteEval : -whiteEval) / ScoreGrain;
}

int32_t BoardEvaluator::PlainMaterialEvaluation(const Board& bd)
{
    int32_t eval = 0;

    for (size_t i = 0; i < Board::BitBoardsCount; ++i)
        eval += CountOnesInBoard(bd.BitBoards[i]) * BasicFigureValues[i];

    return eval;
}

int32_t BoardEvaluator::NaiveEvaluation(const Board& bd)
{
    int32_t eval = 0;

    // iterate through BitBoards
    for (size_t bInd = 0; bInd < Board::BitBoardsCount; ++bInd)
    {
        // extract figures board
        uint64_t figs = bd.BitBoards[bInd];

        // iterate through figures
        while (figs)
        {
            const int figPos = ExtractMsbPos(figs);

            // sum costs offseted by position
            eval += CostsWithPositionsIncluded[bInd][figPos];

            // remove processed figures
            RemovePiece(figs, figPos);
        }
    }

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
    positionEval += _getTaperedEval(bd, phase);

    return materialEval + positionEval;
}

int32_t BoardEvaluator::Evaluation2(Board& bd)
{
    const auto [isSuccess, counts] = _countFigures(bd);
    const int32_t phase = _calcPhase(counts);

    const int32_t materialEval = isSuccess ? _materialTable[_getMaterialBoardIndex(counts)] : _slowMaterialCalculation(counts, phase);
    const int32_t positionEval = _evaluateFields(bd, phase);
    return materialEval + positionEval;
}

std::pair<bool, BoardEvaluator::FigureCountsArrayT> BoardEvaluator::_countFigures(const Board& bd)
{
    // contains information about maximal number of figures on board that we store inside material table
    // exceeding one of those values will result in slow material calculation
    static constexpr size_t OverflowTables[] = { 9, 3, 3, 3, 2 };

    FigureCountsArrayT rv{};
    int overflows = 0;

    for (size_t i = pawnsIndex; i < kingIndex; ++i)
    {
        rv[i] = CountOnesInBoard(bd.BitBoards[i]);
        overflows += rv[i] >= OverflowTables[i];

        rv[i + 5] = CountOnesInBoard(bd.BitBoards[i + bPawnsIndex]);
        overflows += rv[i + 5] >= OverflowTables[i];
    }

    return {overflows == 0, rv};
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
    int32_t actPhase{};

    // calculating game phase
    for (size_t j = 0; j < kingIndex; ++j)
        actPhase += static_cast<int32_t>(figArr[j] + figArr[BlackFigStartIndex + j]) * FigurePhases[j];
    actPhase = (actPhase * MaxTaperedCoef + (FullPhase / 2)) / FullPhase; // FullPhase / 2 ?

    return actPhase;
}

int32_t BoardEvaluator::_getTapperedValue(const int32_t phase, const int32_t midEval, const int32_t endEval)
{
    return (endEval*(MaxTaperedCoef - phase) + midEval*phase) / MaxTaperedCoef;
}

int32_t BoardEvaluator::_getNotTaperedEval(const Board& bd)
{
    int32_t eval{};
    for (size_t i = knightsIndex ; i <= rooksIndex; ++i)
    {
        const uint64_t wFigs = bd.BitBoards[i];
        const uint64_t bFigs = bd.BitBoards[bPawnsIndex + i];

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

int32_t BoardEvaluator::_getTaperedEval(const Board& bd, int32_t phase)
{
    int32_t openinEval{};

    openinEval += _getSimpleFieldEval(
        [=](const int msbInd)
        {
            return BasicBlackPawnPositionValues[msbInd];
        }, bd.BitBoards[wPawnsIndex]
    );

    openinEval -= _getSimpleFieldEval(
        [=](const int msbInd)
        {
            return BasicBlackPawnPositionValues[ConvertToReversedPos(msbInd)];
        }, bd.BitBoards[bPawnsIndex]
    );

    for (size_t i = queensIndex; i <= kingIndex; ++i)
    {
        const uint64_t wFigs = bd.BitBoards[i];
        const uint64_t bFigs = bd.BitBoards[bPawnsIndex + i];

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
        }, bd.BitBoards[wPawnsIndex]
    );

    endEval -= _getSimpleFieldEval(
        [=](const int msbInd)
        {
            return BasicBlackPawnPositionEndValues[ConvertToReversedPos(msbInd)];
        }, bd.BitBoards[bPawnsIndex]
    );

    for (size_t i = queensIndex; i <= kingIndex; ++i)
    {
        const uint64_t wFigs = bd.BitBoards[i];
        const uint64_t bFigs = bd.BitBoards[bPawnsIndex + i];

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

    const auto [whiteMidEval, whiteEndEval, whiteControlledFields, whiteKingInfo] =
        _processPawnEval<WhitePawnMap, NoOp>(bd, whitePinnedFigs, fullMap);

    const auto [blackMidEval, blackEndEval, blackControlledFields, blackKingInfo] =
        _processPawnEval<BlackPawnMap, ConvertToReversedPos>(bd, blackPinnedFigs, fullMap);

    rv.midgameEval = whiteMidEval - blackMidEval;
    rv.endgameEval = whiteEndEval - blackEndEval;
    rv.whiteControlledFields = whiteControlledFields;
    rv.blackControlledFields = blackControlledFields;
    rv.whiteKingSafety = whiteKingInfo;
    rv.blackKingSafety = blackKingInfo;

    return rv;
}

void BoardEvaluator::_evaluateKings(Board& bd, _fieldEvalInfo_t& io)
{
    io.midgameEval += BasicBlackKingPositionValues[ExtractMsbPos(bd.BitBoards[wKingIndex])]
            - BasicBlackKingPositionValues[ConvertToReversedPos(ExtractMsbPos(bd.BitBoards[bKingIndex]))];
    io.endgameEval += BasicBlackKingEndPositionValues[ExtractMsbPos(bd.BitBoards[wKingIndex])]
                    - BasicBlackKingEndPositionValues[ConvertToReversedPos(ExtractMsbPos(bd.BitBoards[bKingIndex]))];

    int32_t bonus{};
    bonus += (io.whiteKingSafety.attackCounts > 2) * (-_kingSafetyValues[io.whiteKingSafety.attackPoints]);
    bonus += (io.blackKingSafety.attackCounts > 2) * (_kingSafetyValues[io.blackKingSafety.attackPoints]);

    int32_t structEval{};

    structEval += KingSafetyEval::EvalKingShelter(bd);
    structEval += KingSafetyEval::EvalKingOpenFiles(bd);

    io.midgameEval += bonus + structEval;
    io.endgameEval += bonus;
}

int32_t BoardEvaluator::_evaluateFields(Board& bd, int32_t phase)
{
    static constexpr void (*EvalFunctions[])(_fieldEvalInfo_t&, Board&, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t)
    {
        _processFigEval<KnightMap, _processKnightEval>,
        _processFigEval<BishopMap, _processBishopEval>,
        _processFigEval<RookMap, _processRookEval>,
        _processFigEval<QueenMap, _processQueenEval>
    };

    _fieldEvalInfo_t result{};
    ChessMechanics mech{bd};

    const uint64_t whiteMap = mech.GetColBitMap(WHITE);
    const uint64_t blackMap = mech.GetColBitMap(BLACK);
    const uint64_t fullMap = blackMap | whiteMap;

    const auto [whitePinnedFigs, un1] = mech.GetPinnedFigsMap<ChessMechanics::PinnedFigGen::WoutAllowedTiles>(WHITE, fullMap);
    const auto [blackPinnedFigs, un2] = mech.GetPinnedFigsMap<ChessMechanics::PinnedFigGen::WoutAllowedTiles>(BLACK, fullMap);

    // ------------------------------
    // Evaluating pawn figures
    // ------------------------------

    const auto pEval = _evaluatePawns(bd, blackPinnedFigs, whitePinnedFigs, fullMap);
    const uint64_t whitePawnControl = pEval.whiteControlledFields;
    const uint64_t blackPawnControl = pEval.blackControlledFields;

    result.midgameEval += pEval.midgameEval;
    result.endgameEval += pEval.endgameEval;

    // ------------------------------
    // Evaluating other pieces
    // ------------------------------

    for(const auto func : EvalFunctions)
        func(result, bd, blackPinnedFigs, whitePinnedFigs, whitePawnControl, blackPawnControl, whiteMap, blackMap);

    // ------------------------------
    // Evaluating king
    // ------------------------------

    _evaluateKings(bd, result);

    const int32_t controlEval = (CountOnesInBoard(result.whiteControlledFields) - CountOnesInBoard(result.blackControlledFields)) * CenterControlBonusPerTile;
    result.midgameEval += controlEval;
    result.endgameEval += controlEval;

    return _getTapperedValue(phase, result.midgameEval, result.endgameEval);
}
