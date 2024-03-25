//
// Created by Jlisowskyy on 2/28/24.
//

#ifndef BOARDEVALUATOR_H
#define BOARDEVALUATOR_H

#include "../Board.h"
#include "../MoveGeneration/ChessMechanics.h"

/*      Collection of evaluation functions
 *
 *  Additional notes:
 *  - currently only full board evaluation is possible
 */

/*          Description of evaluation rules currently used:
 *  - Material values defined by BasicFigureValues
 *  - position values defined CostsWithPositionsIncluded (merged with material values)
 *
 */

// TODO:
// - apply additional prize for exchanges in winning positions

class BoardEvaluator
{
    using FigureCountsArrayT = std::array<size_t, 10>;

public:
    // ------------------------------
    // Class creation
    // ------------------------------

    BoardEvaluator() = default;

    ~BoardEvaluator() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static int32_t DefaultFullEvalFunction(const Board&bd, int color);

    // function uses only material to evaluate passed board
    [[nodiscard]] static int32_t PlainMaterialEvaluation(const Board&bd);

    [[nodiscard]] static int32_t NaiveEvaluation2(const Board&bd);

    [[nodiscard]] static int32_t NaiveEvaluation3(const Board&bd);

    [[nodiscard]] static int32_t Evaluation1(const Board&bd);

    [[nodiscard]] static int32_t Evaluation2(Board&bd);

    // ------------------------------
    // Private class methods
    // ------------------------------

private:
    static int32_t _applyBonusForCoveredPawns(const Board&bd, int32_t eval);

    static std::pair<bool, FigureCountsArrayT> __attribute__((always_inline)) _countFigures(const Board& bd);

    static size_t _getMaterialBoardIndex(const FigureCountsArrayT& counts);

    static int32_t _slowMaterialCalculation(const FigureCountsArrayT& figArr, int32_t actPhase);

    static int32_t _calcPhase(const FigureCountsArrayT&figArr);

    static int32_t _getTapperedValue(int32_t phase, int32_t min, int32_t max);

    template<class EvalF>
    static int32_t _getSimpleFieldEval(EvalF evaluator, uint64_t figs);

    static int32_t _getNotTaperedEval(const Board&bd);

    static int32_t _getTapperedEval(const Board&bd, int32_t phase);

    struct _fieldEvalInfo_t
    {
        int32_t midgameEval;
        int32_t endgameEval;
        uint64_t whiteControlledFields;
        uint64_t blackControlledFields;
    };

    using evalResult = std::tuple<int32_t, int32_t, uint64_t>;

    static _fieldEvalInfo_t _evaluatePawns(Board&bd, uint64_t blackPinnedFigs, uint64_t whitePinnedFigs, uint64_t fullMap);

    template<class MapT, int (*fieldValueAccess)(int msbPos)>
    static evalResult _processPawnEval(Board& bd, uint64_t pinnedFigs, uint64_t fullMap);

    template<class MapT, int (*fieldValueAccess)(int msbPos)>
    struct _processKnightEval
    {
        evalResult operator()(Board& bd, uint64_t pinnedFigs, int col,
        uint64_t enemyControledFieldsByPawns, uint64_t, uint64_t);
    };

    template<class MapT, int (*fieldValueAccess)(int msbPos)>
    struct _processBishopEval
    {
        evalResult operator()(Board& bd, uint64_t pinnedFigs, int col,
        uint64_t enemyControledFieldsByPawns, uint64_t allyMap, uint64_t enemyMap);
    };

    static int32_t _evaluateFields(Board&bd, int32_t phase);

    static int32_t _evaluateStructures(const Board&bd, int32_t phase);

    template<class MapT, template<class, int (*fieldValueAccess)(int msbPos)> class EvalProducerT>
    static _fieldEvalInfo_t _processFigEval(Board&bd, uint64_t blackPinnedFigs, uint64_t whitePinnedFigs
        ,uint64_t whitePawnControlledFields, uint64_t blackPawnControlledFields, uint64_t whiteMap, uint64_t blackMap);

    // ------------------------------
    // Class fields
    // ------------------------------
public:
    using CostArrayT = std::array<std::array<int16_t, Board::BoardFields>, Board::BoardsCount>;

    // for each color and each figure including lack of figure /pawn/knight/bishop/rook/queen/
    static constexpr size_t MaterialTableSize = 9 * 9 * 3 * 3 * 3 * 3 * 3 * 3 * 2 * 2;

    static constexpr size_t BlackPawnCoef = MaterialTableSize / 9;
    static constexpr size_t WhitePawnCoef = BlackPawnCoef / 9;
    static constexpr size_t BlackKnightCoef = WhitePawnCoef / 3;
    static constexpr size_t WhiteKnightCoef = BlackKnightCoef / 3;
    static constexpr size_t BlackBishopCoef = WhiteKnightCoef / 3;
    static constexpr size_t WhiteBishopCoef = BlackBishopCoef / 3;
    static constexpr size_t BlackRookCoef = WhiteBishopCoef / 3;
    static constexpr size_t WhiteRookCoef = BlackRookCoef / 3;
    static constexpr size_t BlackQueenCoef = WhiteRookCoef / 2;
    static constexpr size_t WhiteQueenCoef = BlackQueenCoef / 2;

    static constexpr size_t FigCoefs[]
    {
        WhitePawnCoef,
        WhiteKnightCoef,
        WhiteBishopCoef,
        WhiteRookCoef,
        WhiteQueenCoef,
        BlackPawnCoef,
        BlackKnightCoef,
        BlackBishopCoef,
        BlackRookCoef,
        BlackQueenCoef,
    };

    using MaterialArrayT = std::array<int16_t, MaterialTableSize>;

    static constexpr int16_t CoveredWPawnBonus = 5;
    static constexpr int16_t CoveredBPawnBonus = -5;

    static constexpr int16_t NoPawnsPenalty = -100;

    static constexpr int16_t BishopPairBonus = 50;
    static constexpr int16_t BishopPairDelta = 32;

    static constexpr int16_t KnightPairPenalty = -32;

    static constexpr int16_t RookPairPenalty = -10;

    static constexpr int16_t KnightMobilityBonusMid = 6;
    static constexpr int16_t KnightMobilityBonusEnd = 2;

    static constexpr int16_t BishopMobilityBonusMid = 6;
    static constexpr int16_t BishopMobilityBonusEnd = 2;

    static constexpr int16_t MobilityBonus = 4;

    static constexpr int16_t TrappedPiecePenalty = -50;
    static constexpr int16_t PinnedPawnPenalty = -10;

    static constexpr int16_t TrappedBishopPenalty = -150;

    static constexpr int16_t TrappedRookPenaltyMid = -100;
    static constexpr int16_t TrappedRookPenaltyEnd = -200;

    static constexpr int16_t RookMobilityBonusMid = 2;
    static constexpr int16_t RookMobilityBonusEnd = 6;

    static constexpr int16_t BasicFigureValues[]{
        100, // Pawn
        325, // Knight
        325, // Bishop
        500, // Rook
        975, // Queen
        10000, // king
        -100, // Pawn
        -325, // Knight
        -325, // Bishop
        -500, // Rook
        -975, // Queen
        -10000 // king
    };

    static constexpr int16_t EndGameFigureValues[]{
        130, // Pawn
        340, // Knight
        340, // Bishop
        500, // Rook
        1000, // Queen
    };

    // TODO: What about promotions?

    static constexpr int16_t PawnPhase = 0;
    static constexpr int16_t KnightPhase = 1;
    static constexpr int16_t BishopPhase = 1;
    static constexpr int16_t RookPhase = 2;
    static constexpr int16_t QueenPhase = 4;

    static constexpr int16_t FigurePhases[] = {
        PawnPhase,
        KnightPhase,
        BishopPhase,
        RookPhase,
        QueenPhase
    };

    static constexpr int16_t FullPhase = 4 * KnightPhase + 4 * BishopPhase + 4 * RookPhase + 2 * QueenPhase;
    static constexpr int16_t MaxTapperedCoef = 256;

    static constexpr int16_t BasicBlackPawnPositionValues[]
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        10, 10, 20, 25, 25, 10, 10, 10,
        5, 7, 10, 20, 20, 10, 7, 5,
        5, 5, 5, 0, 0, 5, 5, 5,
        5, 8, 8, -20, -20, 8, 8, 5,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    static constexpr int16_t BasicBlackPawnPositionEndValues[]
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        80, 80, 80, 80, 80, 80, 80, 80,
        40, 40, 50, 60, 60, 50, 40, 40,
        30, 30, 40, 45, 45, 40, 30, 30,
        15, 17, 20, 30, 30, 20, 17, 15,
        -5, -5, -5, 0, 0, -5, -5, -5,
        -20, -20, -30, -30, -30, -20, -20, -20,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    static constexpr int16_t BasicBlackKnightPositionValues[]
    {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50,
    };

    static constexpr int16_t BasicBlackBishopPositionValues[]
    {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 10, 10, 5, 0, -10,
        -10, 5, 5, 10, 10, 5, 5, -10,
        -10, 0, 10, 10, 10, 10, 0, -10,
        -10, 10, 10, 10, 10, 10, 10, -10,
        -10, 5, 0, 0, 0, 0, 5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20,
    };

    static constexpr int16_t BasicBlackRookPositionValues[]
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        5, 10, 10, 10, 10, 10, 10, 5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        0, 0, 5, 5, 5, 5, 0, 0
    };

    static constexpr int16_t BasicBlackQueenPositionValues[]
    {
        -50, -50, -50, -50, -50, -50, -50, -50,
        -30, -30, -30, -30, -30, -30, -30, -30,
        -20, -20, -20, -20, -20, -20, -20, -20,
        -15, -15, -15, -15, -15, -15, -15, -15,
        -10, -5, -5, -5, -5, -5, -5, -10,
        -10, 5, 5, 5, 5, 5, 5, -10,
        -10, 15, 15, 15, 15, 15, 15, -10,
        -20, -10, -10, 30, 30, -10, -10, -20
    };

    static constexpr int16_t BasicBlackQueenEndPositionValues[]
    {
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 5, 5, 5, 0, -10,
        -5, 0, 5, 5, 5, 5, 0, -5,
        0, 0, 5, 5, 5, 5, 0, -5,
        -10, 5, 5, 5, 5, 5, 0, -10,
        -10, 0, 5, 0, 0, 0, 0, -10,
        -20, -10, -10, -5, -5, -10, -10, -20
    };

    static constexpr int16_t BasicBlackKingPositionValues[]{
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        20, 20, 0, 0, 0, 0, 20, 20,
        20, 30, 10, 0, 0, 10, 30, 20
    };

    static constexpr int16_t BasicBlackKingEndPositionValues[]{
        -50, -40, -30, -20, -20, -30, -40, -50,
        -30, -20, -10, 0, 0, -10, -20, -30,
        -30, -10, 20, 30, 30, 20, -10, -30,
        -30, -10, 30, 40, 40, 30, -10, -30,
        -30, -10, 30, 40, 40, 30, -10, -30,
        -30, -10, 20, 30, 30, 20, -10, -30,
        -30, -30, 0, 0, 0, 0, -30, -30,
        -50, -30, -30, -30, -30, -30, -30, -50
    };

    static constexpr FigureCountsArrayT MaterialDrawPositionConstelations[]
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 2, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    };

    static constexpr const int16_t* BasicBlackPositionValues[]{
        BasicBlackPawnPositionValues, BasicBlackKnightPositionValues, BasicBlackBishopPositionValues,
        BasicBlackRookPositionValues, BasicBlackQueenPositionValues, BasicBlackKingPositionValues,
    };

    static constexpr const int16_t* BasicBlackPositionEndValues[]{
        BasicBlackPawnPositionEndValues, BasicBlackKnightPositionValues, BasicBlackBishopPositionValues,
        BasicBlackRookPositionValues, BasicBlackQueenEndPositionValues, BasicBlackKingEndPositionValues,
    };

    static constexpr size_t BlackFigStartIndex = 5;


    static constexpr CostArrayT CostsWithPositionsIncluded = []() constexpr
    {
        CostArrayT arr{};

        constexpr size_t BlackIndex = BLACK * Board::BoardsPerCol;
        for (size_t i = 0; i <= kingIndex; ++i)
        {
            for (int j = 0; j < static_cast<int>(Board::BoardFields); ++j)
                arr[BlackIndex + i][j] = static_cast<int16_t>(-(BasicFigureValues[i] + BasicBlackPositionValues[i][ConvertToReversedPos(j)]));
        }

        constexpr size_t WhiteIndex = WHITE * Board::BoardsPerCol;
        for (size_t i = 0; i <= kingIndex; ++i)
        {
            for (size_t j = 0; j < Board::BoardFields; ++j)
                arr[WhiteIndex + i][j] = static_cast<int16_t>(BasicFigureValues[i] + BasicBlackPositionValues[i][j]);
        }

        return arr;
    }();

    static MaterialArrayT _materialTable;
};

template<class EvalF>
int32_t BoardEvaluator::_getSimpleFieldEval(EvalF evaluator, uint64_t figs)
{
    int32_t eval{};

    while (figs)
    {
        const int figPos = ExtractMsbPos(figs);

        eval += evaluator(figPos);

        // remove processed figures
        figs ^= maxMsbPossible >> figPos;
    }

    return eval;
}

template<class MapT, int(* fieldValueAccess)(int msbPos)>
BoardEvaluator::evalResult BoardEvaluator::_processPawnEval(Board& bd, const uint64_t pinnedFigs, const uint64_t fullMap)
{
    int32_t midEval{};
    int32_t interEval{};
    int32_t endEval{};
    uint64_t pawnControlledFields{};

    uint64_t pinnedPawns = bd.boards[MapT::GetBoardIndex(0)] & pinnedFigs;
    uint64_t unpinnedPawns = bd.boards[MapT::GetBoardIndex(0)] ^ pinnedPawns;

    while(pinnedPawns)
    {
        const int msbPos = ExtractMsbPos(pinnedPawns);
        const uint64_t figMap = maxMsbPossible >> msbPos;

        ChessMechanics mech{bd};
        const uint64_t allowedTiles = mech.GenerateAllowedTilesForPrecisedPinnedFig(figMap, fullMap);
        const uint64_t plainMoves = MapT::GetPlainMoves(figMap, fullMap) & allowedTiles;

        // adding penatly for pinned pawn
        interEval += (plainMoves == 0) * PinnedPawnPenalty;

        // adding pawn control zone to global
        pawnControlledFields |= (allowedTiles & MapT::GetAttackFields(figMap));

        // adding field values
        midEval += BasicBlackPawnPositionValues[fieldValueAccess(msbPos)];
        endEval += BasicBlackPawnPositionEndValues[fieldValueAccess(msbPos)];

        pinnedPawns ^= figMap;
    }

    while(unpinnedPawns)
    {
        const int msbPos = ExtractMsbPos(unpinnedPawns);
        const uint64_t figMap = maxMsbPossible >> msbPos;

        // adding field values
        midEval += BasicBlackPawnPositionValues[fieldValueAccess(msbPos)];
        endEval += BasicBlackPawnPositionEndValues[fieldValueAccess(msbPos)];

        unpinnedPawns ^= figMap;
    }

    // adding controlled fields
    pawnControlledFields |= MapT::GetAttackFields(unpinnedPawns);

    midEval += interEval;
    endEval += interEval;

    return {midEval, endEval, pawnControlledFields};
}

template<class MapT, int(* fieldValueAccess)(int msbPos)>
BoardEvaluator::evalResult BoardEvaluator::_processKnightEval<MapT, fieldValueAccess>::operator()(Board& bd,
    const uint64_t pinnedFigs, const int col, const uint64_t enemyControledFieldsByPawns, const uint64_t, const uint64_t)
{
    int32_t midEval{};
    int32_t interEval{};
    int32_t endEval{};
    uint64_t controlledFields{};

    uint64_t pinnedKnighs = bd.boards[MapT::GetBoardIndex(col)] & pinnedFigs;
    uint64_t unpinnedKnights = bd.boards[MapT::GetBoardIndex(col)] ^ pinnedKnighs;

    const uint64_t safeFields = ~enemyControledFieldsByPawns;

    while(pinnedKnighs)
    {
        const int msbPos = ExtractMsbPos(pinnedKnighs);
        const uint64_t figMap = maxMsbPossible >> msbPos;

        // adding penatly for being pinned
        interEval += TrappedPiecePenalty;

        // adding field values
        interEval += BasicBlackKnightPositionValues[fieldValueAccess(msbPos)];

        pinnedKnighs ^= figMap;
    }

    while(unpinnedKnights)
    {
        const int msbPos = ExtractMsbPos(unpinnedKnights);
        const uint64_t figMap = maxMsbPossible >> msbPos;

        const uint64_t moves = MapT::GetMoves(msbPos);
        const uint64_t safeMoves = moves & safeFields;

        // adding fields to controlled ones
        controlledFields |= moves;

        // adding mobility bonus
        const int mobCount = CountOnesInBoard(safeMoves);
        midEval += mobCount * KnightMobilityBonusMid;
        endEval += mobCount * KnightMobilityBonusEnd;

        // adding field values
        interEval += BasicBlackKnightPositionValues[fieldValueAccess(msbPos)];

        unpinnedKnights ^= figMap;
    }


    midEval += interEval;
    endEval += interEval;

    return {midEval, endEval, controlledFields};
}

template<class MapT, int(* fieldValueAccess)(int msbPos)>
BoardEvaluator::evalResult BoardEvaluator::_processBishopEval<MapT, fieldValueAccess>::operator()(Board& bd,
    const uint64_t pinnedFigs, const int col, const uint64_t enemyControledFieldsByPawns, const uint64_t allyMap, const uint64_t enemyMap)
{
    int32_t midEval{};
    int32_t interEval{};
    int32_t endEval{};
    uint64_t controlledFields{};

    uint64_t pinnedBishops = bd.boards[MapT::GetBoardIndex(col)] & pinnedFigs;
    uint64_t unpinnedBishops = bd.boards[MapT::GetBoardIndex(col)] ^ pinnedBishops;

    const uint64_t safeFields = ~enemyControledFieldsByPawns;
    const uint64_t fullMap = allyMap | enemyMap;

    while(pinnedBishops)
    {
        const int msbPos = ExtractMsbPos(pinnedBishops);
        const uint64_t figMap = maxMsbPossible >> msbPos;

        ChessMechanics mech{bd};
        const uint64_t allowedTiles = mech.GenerateAllowedTilesForPrecisedPinnedFig(figMap, fullMap);

        const uint64_t LegalMoves = allowedTiles & MapT::GetMoves(msbPos, fullMap);

        // trapped penalty
        interEval += TrappedPiecePenalty * (LegalMoves == 0);

        // adding controlle fields
        controlledFields |= LegalMoves;

        // adding mobility bonus
        const int movesCount = CountOnesInBoard(LegalMoves);
        midEval += movesCount * BishopMobilityBonusMid;
        endEval += movesCount * BishopMobilityBonusEnd;

        // adding field values
        interEval += BasicBlackBishopPositionValues[fieldValueAccess(msbPos)];

        pinnedBishops ^= figMap;
    }

    while(unpinnedBishops)
    {
        const int msbPos = ExtractMsbPos(unpinnedBishops);
        const uint64_t figMap = maxMsbPossible >> msbPos;

        const uint64_t moves = MapT::GetMoves(msbPos, fullMap);
        const uint64_t safeMoves = moves & safeFields;

        // adding fields to controlled ones
        controlledFields |= moves;

        // adding mobility bonus
        const int mobCount = CountOnesInBoard(safeMoves);
        midEval += mobCount * KnightMobilityBonusMid;
        endEval += mobCount * KnightMobilityBonusEnd;

        // adding field values
        interEval += BasicBlackBishopPositionValues[fieldValueAccess(msbPos)];

        // adding trapped bishop penalty if bishop is not pinned and therefor has no possible moves
        interEval += TrappedBishopPenalty * ((moves & ~allyMap) == 0);

        unpinnedBishops ^= figMap;
    }


    midEval += interEval;
    endEval += interEval;

    return {midEval, endEval, controlledFields};
}

template<class MapT, template <class, int(* fieldValueAccess)(int msbPos)> class EvalProducerT>
BoardEvaluator::_fieldEvalInfo_t BoardEvaluator::_processFigEval(Board& bd, const uint64_t blackPinnedFigs,
    const uint64_t whitePinnedFigs, const uint64_t whitePawnControlledFields, const uint64_t blackPawnControlledFields,
    const uint64_t whiteMap, const uint64_t blackMap)
{
    _fieldEvalInfo_t rv{};

    const auto [whiteMidEval, whiteEndEval, whiteControlledFields]
            = EvalProducerT<MapT, NoOp>()(bd, whitePinnedFigs, WHITE, blackPawnControlledFields, whiteMap, blackMap);

    const auto [blackMidEval, blackEndEval, blackControlledFields]
            = EvalProducerT<MapT, ConvertToReversedPos>()(bd, blackPinnedFigs, BLACK, whitePawnControlledFields, blackMap, whiteMap);

    rv.midgameEval = whiteMidEval - blackMidEval;
    rv.endgameEval = blackEndEval - whiteEndEval;
    rv.whiteControlledFields = whiteControlledFields;
    rv.blackControlledFields = blackControlledFields;

    return rv;
}

#endif  // BOARDEVALUATOR_H
