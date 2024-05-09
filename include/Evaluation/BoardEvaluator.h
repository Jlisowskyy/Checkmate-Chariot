//
// Created by Jlisowskyy on 2/28/24.
//

#ifndef BOARDEVALUATOR_H
#define BOARDEVALUATOR_H

#include "../Board.h"
#include "../MoveGeneration/ChessMechanics.h"
#include "KingSafetyEval.h"
#include "StructureEvaluator.h"

#include "../MoveGeneration/FileMap.h"

/*      Collection of evaluation functions
 *
 *  Additional notes:
 *  - currently only full board evaluation is possible
 */

// TODO:
// - apply additional prize for exchanges in winning positions
// - what if e.g rook is pinned by rook? (pinning bonuses)
// - DIRECTION MOBILITY BONUSES
// - PAWN PATTERNS
// - ROOK PATTERNS
// - NO CASTLING PENALTIES ?
// - ADD CONTACTLESS KING SAFETY
// - ADD BIGGER KING RING ?

/*              IMPORTANT DEFINITIONS
 * - game phase - is a value that represents how much pieces are left on the board and how much of the board is open.
 *                It is used to interpolate game value between mid-game and endgame evaluation
 *                so to introduce more like continuous evaluation function.
 * - tapered eval - is an evaluation that is based on game phase and is used to interpolate between mid-game and endgame
 * evaluation
 * - king ring - is a set of fields around the king that are used to evaluate king safety
 *
 *
 *
 */

// TODO:
// TODO: there are additional evaluation functions that should be deleted after some tests in the future

/*              Current evaluation elements:
 *  - material table evaluation
 *  - tapered (interpolated) mid-end evaluation
 *  - king's safety evaluation
 *  - mobility evaluation
 *  - center control evaluation
 *  - no pawn penalty
 *  - bishop pair prize
 *  - knight pair penalty
 *  - rook pair penalty
 *  - isolated pawn
 *  - passed pawn
 *  - doubled pawn
 *  - pawn chain
 *  - pinned pieces penalty
 *  - exponential king's safety evaluation values
 *
 * */

class BoardEvaluator
{
    // ------------------------------
    // Inner types
    // ------------------------------

    // array type that is used to store counts of specific figures on the board
    using FigureCountsArrayT = std::array<size_t, 10>;

    using _kingSafetyInfo_t = KingSafetyEval::_kingSafetyInfo_t;

    // simple structure that is used to return positional evaluation results
    struct _fieldEvalInfo_t
    {
        int32_t midgameEval;
        int32_t endgameEval;
        uint64_t whiteControlledFields;
        uint64_t blackControlledFields;
        _kingSafetyInfo_t whiteKingSafety;
        _kingSafetyInfo_t blackKingSafety;
    };

    using evalResult = std::tuple<int32_t, int32_t, uint64_t, _kingSafetyInfo_t>;

    public:
    using CostArrayT = std::array<std::array<int16_t, Board::BitBoardFields>, Board::BitBoardsCount>;

    // for each color and each figure including lack of figure /pawn/knight/bishop/rook/queen/
    static constexpr size_t MaterialTableSize = 9 * 9 * 3 * 3 * 3 * 3 * 3 * 3 * 2 * 2;

    // array that stores material values for each possible board configuration in range given one line above
    using MaterialArrayT = std::array<int16_t, MaterialTableSize>;

    // ------------------------------
    // Class creation
    // ------------------------------

    BoardEvaluator() = default;

    ~BoardEvaluator() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // Wrapper used to run chosen evaluation function
    [[nodiscard]] static int32_t DefaultFullEvalFunction(Board &bd, const int color) __attribute__((always_inline))
    {
        const int whiteEval = Evaluation2(bd);
        return (color == WHITE ? whiteEval : -whiteEval) / ScoreGrain;
    }

    // function uses only material to evaluate passed board
    [[nodiscard]] static int32_t PlainMaterialEvaluation(const Board &bd);

    [[nodiscard]] static int32_t NaiveEvaluation(const Board &bd);

    [[nodiscard]] static int32_t Evaluation1(const Board &bd);

    [[nodiscard]] static int32_t Evaluation2(Board &bd);

    [[nodiscard]] static int32_t InterpGameStage(const Board &bd, int32_t midVal, int32_t endVal);

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    // Function counts all figures on the board and returns array with counts of specific figure types,
    // is mainly used to calculate material table index or simply calculate figure values
    static std::pair<bool, FigureCountsArrayT> _countFigures(const Board &bd) __attribute__((always_inline))
    {
        // contains information about maximal number of figures on board that we store inside material table
        // exceeding one of those values will result in slow material calculation
        static constexpr size_t OverflowTables[] = {9, 3, 3, 3, 2};

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

    // Function calculates material table index based on passed figure counts
    static size_t _getMaterialBoardIndex(const FigureCountsArrayT &counts) __attribute__((always_inline))
    {
        size_t index{};

        for (size_t i = 0; i < counts.size(); ++i) index += counts[i] * FigCoefs[i];

        return index;
    }

    // Function calculates material value based on passed figure counts and actual game phase
    static int32_t _slowMaterialCalculation(const FigureCountsArrayT &figArr, int32_t actPhase);

    // Function calculates game phase based on passed figure counts
    static int32_t _calcPhase(const FigureCountsArrayT &figArr) __attribute__((always_inline))
    {
        int32_t actPhase{};

        // calculating game phase
        for (size_t j = 0; j < kingIndex; ++j)
            actPhase += static_cast<int32_t>(figArr[j] + figArr[BlackFigStartIndex + j]) * FigurePhases[j];
        actPhase = (actPhase * MaxTaperedCoef + (FullPhase / 2)) / FullPhase; // FullPhase / 2 ?

        return actPhase;
    }

    // Function calculates interpolated game value between mig-game and endgame value based on the game phase
    static int32_t _getTapperedValue(int32_t phase, int32_t midEval, int32_t endEval) __attribute__((always_inline))
    {
        return (endEval * (MaxTaperedCoef - phase) + midEval * phase) / MaxTaperedCoef;
    }

    // Input:
    //  - evaluator - function that takes msbInd as an input and returns value of specific figure on specific field
    //  - figs - bitboard with figures to evaluate
    // Output:
    //  - sum of evaluated position of given figure type defined by evaluator
    template <class EvalF> static int32_t _getSimpleFieldEval(EvalF evaluator, uint64_t figs);

    // simply iterates through all figures and positions and evaluates them based on middle game evaluation
    static int32_t _getNotTaperedEval(const Board &bd);

    // simply iterates through all figures and positions and evaluates using tapered eval
    static int32_t _getTaperedEval(const Board &bd, int32_t phase);

    // Function evaluates pawns on the board, based on position and structures, returns values for both colors
    // In short is simple wrapper that runs _processPawnEval for both colors and aggregate results
    static _fieldEvalInfo_t
    _evaluatePawns(Board &bd, uint64_t blackPinnedFigs, uint64_t whitePinnedFigs, uint64_t fullMap);

    // Function iterates through all pawns on given color and evaluates them based on position and structures
    // MapT - map that defines pawn moves and board indexes
    // fieldValueAccess - function used to transform msbPos to
    //                      index in field->value table is used to flip one board to others board values
    // Currently calculated patterns:
    //  - doubled pawns
    //  - isolated pawns
    //  - passed pawns
    template <class MapT, int (*fieldValueAccess)(int msbPos)>
    static evalResult _processPawnEval(Board &bd, uint64_t pinnedFigs, uint64_t fullMap);

    // Function performs king position evaluation
    static void _evaluateKings(Board &bd, _fieldEvalInfo_t &io);

    // Function performs positional evaluation of the whole board, simply iterates through all figure types and append
    // the results to the output. Output is tapered based on given phase.
    static int32_t _evaluateFields(Board &bd, int32_t phase);

    // Function takes as a template argument Map of given figure and one of belows function that is used to evaluate
    // specific figure on both colors and append the result to given out object.
    // PawnControlledFields - means fields that pawns are attacking
    // ColorBitMap - map of figures with given color
    template <class MapT, template <class, int (*fieldValueAccess)(int msbPos)> class EvalProducerT>
    static void _processFigEval(
        _fieldEvalInfo_t &out, Board &bd, uint64_t blackPinnedFigsBitMap, uint64_t whitePinnedFigsBitMap,
        uint64_t whitePawnControlledFieldsBitMap, uint64_t blackPawnControlledFieldsBitMap, uint64_t whiteBitMap,
        uint64_t blackBitMap
    );

    // ------------------------------------------
    // Similar figure types positional eval
    // ------------------------------------------

    /*
     * All four functions below have similar call type to simplify their usage,
     *  I decided that for every figure type there should be specific function that will be used to evaluate
     *  to introduce ease of adding new structures and patterns processing.
     */

    template <class MapT, int (*fieldValueAccess)(int msbPos)> struct _processKnightEval
    {
        evalResult
        operator()(Board &bd, uint64_t pinnedFigs, int col, uint64_t enemyControlledFieldsByPawns, uint64_t, uint64_t);
    };

    template <class MapT, int (*fieldValueAccess)(int msbPos)> struct _processBishopEval
    {
        evalResult operator()(
            Board &bd, uint64_t pinnedFigs, int col, uint64_t enemyControlledFieldsByPawns, uint64_t allyMap,
            uint64_t enemyMap
        );
    };

    template <class MapT, int (*fieldValueAccess)(int msbPos)> struct _processRookEval
    {
        evalResult operator()(
            Board &bd, uint64_t pinnedFigs, int col, uint64_t enemyControlledFieldsByPawns, uint64_t allyMap,
            uint64_t enemyMap
        );
    };

    template <class MapT, int (*fieldValueAccess)(int msbPos)> struct _processQueenEval
    {
        evalResult operator()(
            Board &bd, uint64_t pinnedFigs, int col, uint64_t enemyControlledFieldsByPawns, uint64_t allyMap,
            uint64_t enemyMap
        );
    };

    // ------------------------------
    // Class fields
    // ------------------------------

    public:
    // The value below ensures that resulted evaluation score will be rounded  value divisible by ScoreGrain
    static constexpr int32_t ScoreGrain = 8;

    // All belows values are used to calculate material table index, they symbolize coefficients for each figure type
    static constexpr size_t BlackPawnCoef   = MaterialTableSize / 9;
    static constexpr size_t WhitePawnCoef   = BlackPawnCoef / 9;
    static constexpr size_t BlackKnightCoef = WhitePawnCoef / 3;
    static constexpr size_t WhiteKnightCoef = BlackKnightCoef / 3;
    static constexpr size_t BlackBishopCoef = WhiteKnightCoef / 3;
    static constexpr size_t WhiteBishopCoef = BlackBishopCoef / 3;
    static constexpr size_t BlackRookCoef   = WhiteBishopCoef / 3;
    static constexpr size_t WhiteRookCoef   = BlackRookCoef / 3;
    static constexpr size_t BlackQueenCoef  = WhiteRookCoef / 2;
    static constexpr size_t WhiteQueenCoef  = BlackQueenCoef / 2;

    // Coefficients for each figure type stored inside the array for easier access
    static constexpr size_t FigCoefs[]{
        WhitePawnCoef, WhiteKnightCoef, WhiteBishopCoef, WhiteRookCoef, WhiteQueenCoef,
        BlackPawnCoef, BlackKnightCoef, BlackBishopCoef, BlackRookCoef, BlackQueenCoef,
    };

    // This penalty is given the player has no pawns on the board
    // Reasoning:
    //      This is usually bad position when no pawns are on the board, especially in the late game due to
    //      no possibility of promotion and no pawn structure to defend the king
    static constexpr int16_t NoPawnsPenalty = -100;

    // This prize is given if player has bishop pair
    // Reasoning:
    //      Bishop pair is usually considered as a good thing, because bishops are long range pieces, so when there
    //      is only small amount of pawns on the map, they introduce a big pressure on the enemy
    static constexpr int16_t BishopPairBonus = 50;

    // This value is used to decrease BishopPairBonus by the delta,
    // which scales accordingly to pawns on the board.
    // The formula looks like: BishopPairBonus - (BishopPairDelta - 2 * pawnsCount)
    // Reasoning:
    //      Bishop pair should gain more value when there are fewer pawns on the board, because they are more
    //      capable of controlling the board
    static constexpr int16_t BishopPairDelta = 32;

    // Denotes the penalty for having two knights on the board, again it scales with the number of pawns on the board
    // accordingly to the formula: KnightPairPenalty + 2 * pawnsCount
    // Reasoning:
    //      Knights are short range pieces, so they are less effective when there are fewer pawns on the board
    static constexpr int16_t KnightPairPenalty = -32;

    // Denotes the penalty for having two rooks on the board.
    // Reasoning:
    //      We should try to guide the engine to exchange one of the rooks, because in most cases this allows to create
    //      more open files for the remaining rook and guide it towards the end game
    static constexpr int16_t RookPairPenalty = -10;

    // Values used to calculate mobility bonus for each figure type at the end-game and mid-game
    // Reasoning:
    //      We want to maximize amount of possible moves our figure can make a try to squeeze the enemy as much as we
    //      can
    static constexpr int16_t KnightMobilityBonusMid = 6;
    static constexpr int16_t KnightMobilityBonusEnd = 2;

    static constexpr int16_t BishopMobilityBonusMid = 6;
    static constexpr int16_t BishopMobilityBonusEnd = 2;

    static constexpr int16_t RookMobilityBonusMid = 2;
    static constexpr int16_t RookMobilityBonusEnd = 6;

    static constexpr int16_t QueenMobilityBonusMid = 1;
    static constexpr int16_t QueenMobilityBonusEnd = 8;

    // universal mobility bonus for all figures not mentioned above
    static constexpr int16_t MobilityBonus = 4;

    // Decreases the value of pinned figures (rooks, bishops and knights) that have no possibility to move
    // Reasoning:
    //      Trapped peace that cannot do any move is usually vulnerable to be killed in close sequence of moves
    static constexpr int16_t TrappedPiecePenalty = -20;
    static constexpr int16_t PinnedPawnPenalty   = -10;

    // Values below is used to apply bonus per each tile that is controlled on the board center by given color
    // Reasoning:
    //      Center of the board is the most important part of the map so maximizing the control of it may be a good idea
    static constexpr int16_t CenterControlBonusPerTile = 2;

    // 4x4 mask on the center board used to evaluate center control
    static constexpr uint64_t CenterFieldsMap = []() constexpr
    {
        constexpr uint64_t mask = GenMask(0, 63, 8) | GenMask(1, 63, 8) | GenMask(6, 63, 8) | GenMask(7, 63, 8) |
                                  GenMask(0, 8, 1) | GenMask(8, 16, 1) | GenMask(48, 56, 1) | GenMask(56, 64, 1);
        return ~mask;
    }();

    // values used to calculate material value of given board at the mid-game stage
    static constexpr int16_t BasicFigureValues[]{
        100,   // Pawn
        325,   // Knight
        325,   // Bishop
        500,   // Rook
        975,   // Queen
        10000, // king
        -100,  // Pawn
        -325,  // Knight
        -325,  // Bishop
        -500,  // Rook
        -975,  // Queen
        -10000 // king
    };

    // values that are used to calculate material value of given board at the end-game stage
    static constexpr int16_t EndGameFigureValues[]{
        130,  // Pawn
        340,  // Knight
        340,  // Bishop
        500,  // Rook
        1000, // Queen
    };

    // Weights below are usd to calculate game phase based on the number of figures on the board
    static constexpr int16_t PawnPhase   = 0;
    static constexpr int16_t KnightPhase = 1;
    static constexpr int16_t BishopPhase = 1;
    static constexpr int16_t RookPhase   = 2;
    static constexpr int16_t QueenPhase  = 4;

    static constexpr int16_t FigurePhases[] = {PawnPhase, KnightPhase, BishopPhase, RookPhase, QueenPhase};

    // Full phase based on available figure counts
    static constexpr int16_t FullPhase = 4 * KnightPhase + 4 * BishopPhase + 4 * RookPhase + 2 * QueenPhase;

    // Maximal value to which the phase can be scaled
    static constexpr int16_t MaxTaperedCoef = 256;

    // ----------------------------------------------
    // Figure-Position bonuses/penalties tables
    // ----------------------------------------------

    static constexpr int16_t BasicBlackPawnPositionValues[]{0,   0,  0,  0,   0,  0,  0,   0,  50, 50, 50, 50, 50,
                                                            50,  50, 50, 10,  10, 20, 30,  30, 20, 10, 10, 5,  5,
                                                            10,  25, 25, 10,  5,  5,  0,   -5, -5, 20, 20, -5, -5,
                                                            0,   5,  -5, -10, 0,  0,  -10, -5, 5,  10, 15, 15, -20,
                                                            -20, 15, 15, 10,  0,  0,  0,   0,  0,  0,  0,  0};

    static constexpr int16_t BasicBlackPawnPositionEndValues[]{
        0,  0,  0,  0,  0,   0,   0,   0,   80,  80,  80,  80,  80, 80, 80, 80, 40, 40, 50, 60, 60, 50,
        40, 40, 30, 30, 40,  45,  45,  40,  30,  30,  15,  17,  20, 30, 30, 20, 17, 15, -5, -5, -5, 0,
        0,  -5, -5, -5, -20, -20, -30, -30, -30, -20, -20, -20, 0,  0,  0,  0,  0,  0,  0,  0
    };

    static constexpr int16_t BasicBlackKnightPositionValues[]{
        -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,   0,   -20, -40, -30, 0,   10,  15,  15, 10,
        0,   -30, -30, 5,   15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,   -30, -30, 5,   10, 15,
        15,  10,  5,   -30, -40, -20, 0,   5,   5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50,
    };

    static constexpr int16_t BasicBlackBishopPositionValues[]{
        -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,   0,   0,   -10, -10, 0,   5,   10,  10, 5,
        0,   -10, -10, 5,   5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,   -10, -10, 10,  10, 10,
        10,  10,  10,  -10, -10, 5,   0,   0,   0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20,
    };

    static constexpr int16_t BasicBlackRookPositionValues[]{0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
                                                            -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
                                                            -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
                                                            -5, 0, 0, 0, 0, 0, 0, -5, 0,  0,  5,  5,  5,  5,  0,  0};

    static constexpr int16_t BasicBlackQueenPositionValues[]{
        -110, -110, -110, -110, -110, -110, -110, -110, -80, -80, -80, -80, -80, -80, -80, -80,
        -60,  -60,  -60,  -60,  -60,  -60,  -60,  -60,  -45, -45, -45, -45, -45, -45, -45, -45,
        -20,  -15,  -15,  -15,  -15,  -15,  -15,  -20,  -10, 5,   5,   5,   5,   5,   5,   -10,
        -10,  15,   15,   15,   15,   15,   15,   -10,  -20, -10, -10, 30,  30,  -10, -10, -20
    };

    static constexpr int16_t BasicBlackQueenEndPositionValues[]{
        -20, -10, -10, -5,  -5,  -10, -10, -20, -10, 0,  0, 0,   0,   0,   0,   -10, -10, 0,   5,   5,  5, 5,
        0,   -10, -5,  0,   5,   5,   5,   5,   0,   -5, 0, 0,   5,   5,   5,   5,   0,   -5,  -10, 5,  5, 5,
        5,   5,   0,   -10, -10, 0,   5,   0,   0,   0,  0, -10, -20, -10, -10, -5,  -5,  -10, -10, -20
    };

    static constexpr int16_t BasicBlackKingPositionValues[]{
        -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40,
        -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30, -20, -10, -20, -20, -20,
        -20, -20, -20, -10, 20,  20,  0,   0,   0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20
    };

    static constexpr int16_t BasicBlackKingEndPositionValues[]{
        -50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0,   0,   -10, -20, -30, -30, -10, 20,  30,  30, 20,
        -10, -30, -30, -10, 30,  40,  40,  30,  -10, -30, -30, -10, 30,  40,  40,  30,  -10, -30, -30, -10, 20, 30,
        30,  20,  -10, -30, -30, -30, 0,   0,   0,   0,   -30, -30, -50, -30, -30, -30, -30, -30, -30, -50
    };

    static constexpr const int16_t *BasicBlackPositionValues[]{
        BasicBlackPawnPositionValues, BasicBlackKnightPositionValues, BasicBlackBishopPositionValues,
        BasicBlackRookPositionValues, BasicBlackQueenPositionValues,  BasicBlackKingPositionValues,
    };

    static constexpr const int16_t *BasicBlackPositionEndValues[]{
        BasicBlackPawnPositionEndValues, BasicBlackKnightPositionValues,   BasicBlackBishopPositionValues,
        BasicBlackRookPositionValues,    BasicBlackQueenEndPositionValues, BasicBlackKingEndPositionValues,
    };

    // This table below is generated to simplify usage of above's tables. It is only used in naive evaluation.
    static constexpr CostArrayT CostsWithPositionsIncluded = []() constexpr
    {
        CostArrayT arr{};

        constexpr size_t BlackIndex = BLACK * Board::BitBoardsPerCol;
        for (size_t i = 0; i <= kingIndex; ++i)
        {
            for (int j = 0; j < static_cast<int>(Board::BitBoardFields); ++j)
                arr[BlackIndex + i][j] =
                    static_cast<int16_t>(-(BasicFigureValues[i] + BasicBlackPositionValues[i][ConvertToReversedPos(j)])
                    );
        }

        constexpr size_t WhiteIndex = WHITE * Board::BitBoardsPerCol;
        for (size_t i = 0; i <= kingIndex; ++i)
        {
            for (size_t j = 0; j < Board::BitBoardFields; ++j)
                arr[WhiteIndex + i][j] = static_cast<int16_t>(BasicFigureValues[i] + BasicBlackPositionValues[i][j]);
        }

        return arr;
    }();

    // ------------------------------
    // Material table
    // ------------------------------

    // This table stores all position, which are 100% draw
    // TODO: currently it stores only 7 patterns and the eval returns 0 for all of them, which is not the best idea
    // TODO: there should be clear indication that this is draw, because besides that position eval is also summed at
    // the end
    static constexpr FigureCountsArrayT MaterialDrawPositionConstelations[]{
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 2, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    };

    static constexpr size_t BlackFigStartIndex = 5;
    static MaterialArrayT _materialTable;
};

template <class EvalF> int32_t BoardEvaluator::_getSimpleFieldEval(EvalF evaluator, uint64_t figs)
{
    int32_t eval{};

    while (figs)
    {
        const int figPos = ExtractMsbPos(figs);

        eval += evaluator(figPos);

        RemovePiece(figs, figPos);
    }

    return eval;
}

template <class MapT, int (*fieldValueAccess)(int msbPos)>
BoardEvaluator::evalResult
BoardEvaluator::_processPawnEval(Board &bd, const uint64_t pinnedFigs, const uint64_t fullMap)
{
    int32_t midEval{};
    int32_t interEval{};
    int32_t endEval{};
    uint64_t pawnControlledFields{};
    _kingSafetyInfo_t kInfo{};

    uint64_t pinnedPawns   = bd.BitBoards[MapT::GetBoardIndex(0)] & pinnedFigs;
    uint64_t unpinnedPawns = bd.BitBoards[MapT::GetBoardIndex(0)] ^ pinnedPawns;

    // generating king ring
    const uint64_t kingRing = KingSafetyEval::GetSafetyFields(bd, MapT::GetColor());

    while (pinnedPawns)
    {
        const int msbPos      = ExtractMsbPos(pinnedPawns);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        ChessMechanics mech{bd};
        const uint64_t allowedTiles = mech.GenerateAllowedTilesForPrecisedPinnedFig(figMap, fullMap);
        const uint64_t plainMoves   = FilterMoves(MapT::GetPlainMoves(figMap, fullMap), allowedTiles);

        // adding penatly for pinned pawn
        interEval += (plainMoves == 0) * PinnedPawnPenalty;

        // adding pawn control zone to global
        pawnControlledFields |= FilterMoves(MapT::GetAttackFields(figMap), allowedTiles);

        // adding doubled pawn penalty
        interEval +=
            StructureEvaluator::EvalDoubledPawn(bd.BitBoards[MapT::GetBoardIndex(0)], msbPos, MapT::GetColor());

        // adding isolated pawn penalty
        interEval += StructureEvaluator::EvalIsolatedPawn(bd.BitBoards[MapT::GetBoardIndex(0)], msbPos);

        // adding passed pawn penalty
        interEval += StructureEvaluator::SimplePassedPawn(
            bd.BitBoards[MapT::GetEnemyPawnBoardIndex()], msbPos, MapT::GetColor()
        );

        // adding field values
        midEval += BasicBlackPawnPositionValues[fieldValueAccess(msbPos)];
        endEval += BasicBlackPawnPositionEndValues[fieldValueAccess(msbPos)];

        RemovePiece(pinnedPawns, figMap);
    }

    while (unpinnedPawns)
    {
        const int msbPos      = ExtractMsbPos(unpinnedPawns);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        const uint64_t attackFields = MapT::GetAttackFields(figMap);

        KingSafetyEval::UpdateKingAttacks(kInfo, attackFields, kingRing, KingSafetyEval::KingMinorPieceAttackPoints);

        // adding field values
        midEval += BasicBlackPawnPositionValues[fieldValueAccess(msbPos)];
        endEval += BasicBlackPawnPositionEndValues[fieldValueAccess(msbPos)];

        // adding doubled pawn penalty
        interEval +=
            StructureEvaluator::EvalDoubledPawn(bd.BitBoards[MapT::GetBoardIndex(0)], msbPos, MapT::GetColor());

        // adding isolated pawn penalty
        interEval += StructureEvaluator::EvalIsolatedPawn(bd.BitBoards[MapT::GetBoardIndex(0)], msbPos);

        // adding passed pawn penalty
        interEval += StructureEvaluator::SimplePassedPawn(
            bd.BitBoards[MapT::GetEnemyPawnBoardIndex()], msbPos, MapT::GetColor()
        );

        RemovePiece(unpinnedPawns, figMap);
    }

    // adding controlled fields
    pawnControlledFields |= MapT::GetAttackFields(unpinnedPawns);

    interEval += StructureEvaluator::EvalPawnChain(bd.BitBoards[MapT::GetBoardIndex(0)], pawnControlledFields);

    midEval += interEval;
    endEval += interEval;

    return {midEval, endEval, pawnControlledFields, kInfo};
}

template <class MapT, int (*fieldValueAccess)(int msbPos)>
BoardEvaluator::evalResult BoardEvaluator::_processKnightEval<MapT, fieldValueAccess>::operator()(
    Board &bd, const uint64_t pinnedFigs, const int col, const uint64_t enemyControlledFieldsByPawns, const uint64_t,
    const uint64_t
)
{
    int32_t midEval{};
    int32_t interEval{};
    int32_t endEval{};
    uint64_t controlledFields{};
    _kingSafetyInfo_t kInfo{};

    uint64_t pinnedKnights   = bd.BitBoards[MapT::GetBoardIndex(col)] & pinnedFigs;
    uint64_t unpinnedKnights = bd.BitBoards[MapT::GetBoardIndex(col)] ^ pinnedKnights;

    const uint64_t safeFields = ~enemyControlledFieldsByPawns;

    // generating king ring
    const uint64_t kingRing = KingSafetyEval::GetSafetyFields(bd, col);

    while (pinnedKnights)
    {
        const int msbPos      = ExtractMsbPos(pinnedKnights);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        // adding penalty for being pinned
        interEval += TrappedPiecePenalty;

        // adding field values
        interEval += BasicBlackKnightPositionValues[fieldValueAccess(msbPos)];

        RemovePiece(pinnedKnights, figMap);
    }

    while (unpinnedKnights)
    {
        const int msbPos      = ExtractMsbPos(unpinnedKnights);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        const uint64_t moves     = MapT::GetMoves(msbPos);
        const uint64_t safeMoves = FilterMoves(moves, safeFields);

        // adding fields to controlled ones
        controlledFields |= moves;

        // adding mobility bonus
        const int mobCount = CountOnesInBoard(safeMoves);
        midEval += mobCount * KnightMobilityBonusMid;
        endEval += mobCount * KnightMobilityBonusEnd;

        // adding king attack info
        KingSafetyEval::UpdateKingAttacks(kInfo, moves, kingRing, KingSafetyEval::KingMinorPieceAttackPoints);

        // adding field values
        interEval += BasicBlackKnightPositionValues[fieldValueAccess(msbPos)];

        RemovePiece(unpinnedKnights, figMap);
    }

    midEval += interEval;
    endEval += interEval;

    return {midEval, endEval, controlledFields, kInfo};
}

template <class MapT, int (*fieldValueAccess)(int msbPos)>
BoardEvaluator::evalResult BoardEvaluator::_processBishopEval<MapT, fieldValueAccess>::operator()(
    Board &bd, const uint64_t pinnedFigs, const int col, const uint64_t enemyControlledFieldsByPawns,
    const uint64_t allyMap, const uint64_t enemyMap
)
{
    int32_t midEval{};
    int32_t interEval{};
    int32_t endEval{};
    uint64_t controlledFields{};
    _kingSafetyInfo_t kInfo{};

    uint64_t pinnedBishops   = bd.BitBoards[MapT::GetBoardIndex(col)] & pinnedFigs;
    uint64_t unpinnedBishops = bd.BitBoards[MapT::GetBoardIndex(col)] ^ pinnedBishops;

    const uint64_t safeFields = ~enemyControlledFieldsByPawns;
    const uint64_t fullMap    = allyMap | enemyMap;

    // generating king ring
    const uint64_t kingRing = KingSafetyEval::GetSafetyFields(bd, col);

    while (pinnedBishops)
    {
        const int msbPos      = ExtractMsbPos(pinnedBishops);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        ChessMechanics mech{bd};
        const uint64_t allowedTiles = mech.GenerateAllowedTilesForPrecisedPinnedFig(figMap, fullMap);

        const uint64_t LegalMoves = FilterMoves(MapT::GetMoves(msbPos, fullMap), allowedTiles);

        // trapped penalty
        interEval += TrappedPiecePenalty * (LegalMoves == 0);

        // adding controlled fields
        controlledFields |= LegalMoves;

        // adding mobility bonus
        const int movesCount = CountOnesInBoard(LegalMoves & safeFields);
        midEval += movesCount * BishopMobilityBonusMid;
        endEval += movesCount * BishopMobilityBonusEnd;

        RemovePiece(pinnedBishops, figMap);
    }

    while (unpinnedBishops)
    {
        const int msbPos      = ExtractMsbPos(unpinnedBishops);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        const uint64_t moves     = MapT::GetMoves(msbPos, fullMap);
        const uint64_t safeMoves = FilterMoves(moves, safeFields);

        // adding fields to controlled ones
        controlledFields |= moves;

        // adding mobility bonus
        const int movesCount = CountOnesInBoard(safeMoves);
        midEval += movesCount * BishopMobilityBonusMid;
        endEval += movesCount * BishopMobilityBonusEnd;

        // adding king attack info
        KingSafetyEval::UpdateKingAttacks(kInfo, moves, kingRing, KingSafetyEval::KingMinorPieceAttackPoints);

        RemovePiece(unpinnedBishops, figMap);
    }

    midEval += interEval;
    endEval += interEval;

    return {midEval, endEval, controlledFields, kInfo};
}

template <class MapT, int (*fieldValueAccess)(int msbPos)>
BoardEvaluator::evalResult BoardEvaluator::_processRookEval<MapT, fieldValueAccess>::operator()(
    Board &bd, const uint64_t pinnedFigs, const int col, const uint64_t enemyControlledFieldsByPawns,
    const uint64_t allyMap, const uint64_t enemyMap
)
{
    int32_t midEval{};
    int32_t interEval{};
    int32_t endEval{};
    uint64_t controlledFields{};
    _kingSafetyInfo_t kInfo{};

    uint64_t pinnedRooks   = bd.BitBoards[MapT::GetBoardIndex(col)] & pinnedFigs;
    uint64_t unpinnedRooks = bd.BitBoards[MapT::GetBoardIndex(col)] ^ pinnedRooks;

    const uint64_t safeFields = ~enemyControlledFieldsByPawns;
    const uint64_t fullMap    = allyMap | enemyMap;

    // generating king ring
    const uint64_t kingRing = KingSafetyEval::GetSafetyFields(bd, col);

    while (pinnedRooks)
    {
        const int msbPos      = ExtractMsbPos(pinnedRooks);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        ChessMechanics mech{bd};
        const uint64_t allowedTiles = mech.GenerateAllowedTilesForPrecisedPinnedFig(figMap, fullMap);

        const uint64_t LegalMoves = FilterMoves(MapT::GetMoves(msbPos, fullMap), allowedTiles);

        // trapped penalty
        interEval += TrappedPiecePenalty * (LegalMoves == 0);

        // open file bonus
        interEval += StructureEvaluator::EvalRookOnOpenFile(bd, msbPos, col);

        // adding controlle fields
        controlledFields |= LegalMoves;

        // adding mobility bonus
        const int movesCount = CountOnesInBoard(LegalMoves & safeFields);
        midEval += movesCount * RookMobilityBonusMid;
        endEval += movesCount * RookMobilityBonusEnd;

        RemovePiece(pinnedRooks, figMap);
    }

    while (unpinnedRooks)
    {
        const int msbPos      = ExtractMsbPos(unpinnedRooks);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        const uint64_t moves     = MapT::GetMoves(msbPos, fullMap);
        const uint64_t safeMoves = FilterMoves(moves, safeFields);

        // adding fields to controlled ones
        controlledFields |= moves;

        // adding mobility bonus
        const int32_t movesCount = CountOnesInBoard(safeMoves);
        midEval += movesCount * RookMobilityBonusMid;
        endEval += movesCount * RookMobilityBonusEnd;

        // open file bonus
        interEval += StructureEvaluator::EvalRookOnOpenFile(bd, msbPos, col);

        // adding king attack info
        KingSafetyEval::UpdateKingAttacks(kInfo, moves, kingRing, KingSafetyEval::KingRookAttackPoints);

        RemovePiece(unpinnedRooks, figMap);
    }

    midEval += interEval;
    endEval += interEval;

    return {midEval, endEval, controlledFields, kInfo};
}

template <class MapT, int (*fieldValueAccess)(int msbPos)>
BoardEvaluator::evalResult BoardEvaluator::_processQueenEval<MapT, fieldValueAccess>::operator()(
    Board &bd, const uint64_t pinnedFigs, const int col, const uint64_t enemyControlledFieldsByPawns,
    const uint64_t allyMap, const uint64_t enemyMap
)
{
    int32_t midEval{};
    int32_t endEval{};
    uint64_t controlledFields{};
    _kingSafetyInfo_t kInfo{};

    uint64_t pinnedQueens   = bd.BitBoards[MapT::GetBoardIndex(col)] & pinnedFigs;
    uint64_t unpinnedQueens = bd.BitBoards[MapT::GetBoardIndex(col)] ^ pinnedQueens;

    const uint64_t safeFields = ~enemyControlledFieldsByPawns;
    const uint64_t fullMap    = allyMap | enemyMap;

    // generating king ring
    const uint64_t kingRing = KingSafetyEval::GetSafetyFields(bd, col);

    while (pinnedQueens)
    {
        const int msbPos      = ExtractMsbPos(pinnedQueens);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        ChessMechanics mech{bd};
        const uint64_t allowedTiles = mech.GenerateAllowedTilesForPrecisedPinnedFig(figMap, fullMap);

        const uint64_t LegalMoves = FilterMoves(MapT::GetMoves(msbPos, fullMap), allowedTiles);

        // adding controlled fields
        controlledFields |= LegalMoves;

        // adding mobility bonus
        const int movesCount = CountOnesInBoard(FilterMoves(LegalMoves, safeFields));
        midEval += movesCount * QueenMobilityBonusMid;
        endEval += movesCount * QueenMobilityBonusEnd;

        // adding positional field values
        midEval += BasicBlackQueenPositionValues[fieldValueAccess(msbPos)];

        RemovePiece(pinnedQueens, figMap);
    }

    while (unpinnedQueens)
    {
        const int msbPos      = ExtractMsbPos(unpinnedQueens);
        const uint64_t figMap = ConvertMsbPosToBitMap(msbPos);

        const uint64_t moves     = MapT::GetMoves(msbPos, fullMap);
        const uint64_t safeMoves = FilterMoves(moves, safeFields);

        // adding fields to controlled ones
        controlledFields |= moves;

        // adding mobility bonus
        const int32_t movesCount = CountOnesInBoard(safeMoves);
        midEval += movesCount * QueenMobilityBonusMid;
        endEval += movesCount * QueenMobilityBonusEnd;

        // adding positional field values
        midEval += BasicBlackQueenPositionValues[fieldValueAccess(msbPos)];

        // adding king attack info
        KingSafetyEval::UpdateKingAttacks(kInfo, moves, kingRing, KingSafetyEval::KingQueenAttackPoints);

        RemovePiece(unpinnedQueens, figMap);
    }

    return {midEval, endEval, controlledFields, kInfo};
}

template <class MapT, template <class, int (*fieldValueAccess)(int msbPos)> class EvalProducerT>
void BoardEvaluator::_processFigEval(
    _fieldEvalInfo_t &out, Board &bd, const uint64_t blackPinnedFigsBitMap, const uint64_t whitePinnedFigsBitMap,
    const uint64_t whitePawnControlledFieldsBitMap, const uint64_t blackPawnControlledFieldsBitMap,
    const uint64_t whiteBitMap, const uint64_t blackBitMap
)
{
    const auto [whiteMidEval, whiteEndEval, whiteControlledFields, wKInfo] = EvalProducerT<MapT, NoOp>()(
        bd, whitePinnedFigsBitMap, WHITE, blackPawnControlledFieldsBitMap, whiteBitMap, blackBitMap
    );

    const auto [blackMidEval, blackEndEval, blackControlledFields, bKInfo] =
        EvalProducerT<MapT, ConvertToReversedPos>()(
            bd, blackPinnedFigsBitMap, BLACK, whitePawnControlledFieldsBitMap, blackBitMap, whiteBitMap
        );

    out.midgameEval += whiteMidEval - blackMidEval;
    out.endgameEval += blackEndEval - whiteEndEval;
    out.whiteControlledFields |= whiteControlledFields;
    out.blackControlledFields |= blackControlledFields;
    out.whiteKingSafety.attackCounts += wKInfo.attackCounts;
    out.whiteKingSafety.attackPoints += wKInfo.attackPoints;
    out.blackKingSafety.attackCounts += bKInfo.attackCounts;
    out.blackKingSafety.attackPoints += bKInfo.attackPoints;
}

#endif // BOARDEVALUATOR_H
