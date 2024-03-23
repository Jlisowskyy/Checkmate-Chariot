//
// Created by Jlisowskyy on 2/28/24.
//

#ifndef BOARDEVALUATOR_H
#define BOARDEVALUATOR_H

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/KingMap.h"

/*      Collection of evaluation function
 *
 *  Additional notes:
 *  - currently only full board evalution is possible
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

    [[nodiscard]] static int32_t DefaultFullEvalFunction(const Board&bd, const int color)
    {
        const int whiteEval = Evaluation1(bd);
        return color == WHITE ? whiteEval : -whiteEval;
    }

    // function uses only material to evaluate passed board
    [[nodiscard]] static int32_t PlainMaterialEvaluation(const Board&bd);

    [[nodiscard]] static int32_t NaiveEvaluation2(const Board&bd);

    [[nodiscard]] static int32_t NaiveEvaluation3(const Board&bd);

    [[nodiscard]] static int32_t Evaluation1(const Board&bd);

    // ------------------------------
    // Private class methods
    // ------------------------------

private:
    static int32_t _applyBonusForCoveredPawns(const Board&bd, int32_t eval);

    static std::pair<bool, FigureCountsArrayT> __attribute__((always_inline))  _countFigures(const Board& bd);

    static size_t _getMaterialBoardIndex(const FigureCountsArrayT& counts);

    static int16_t _slowMaterialCalculation(const FigureCountsArrayT& figArr, int actPhase);

    static int16_t _calcPhase(const FigureCountsArrayT& figArr);

    static int16_t _getTapperedValue(int16_t phase, int16_t min, int16_t max);

    template<class EvalF>
    static int16_t _getSimpleFieldEval(EvalF evaluator, uint64_t figs)
    {
        int16_t eval{};

        while (figs)
        {
            const int figPos = ExtractMsbPos(figs);

            eval += evaluator(figPos);

            // remove processed figures
            figs ^= maxMsbPossible >> figPos;
        }

        return eval;
    }

    static int16_t _getNotTaperedEval(const Board& bd);
    static int16_t _getTapperedEval(const Board& bd, int16_t phase);

    // ------------------------------
    // Class fields
    // ------------------------------
public:
    using CostArrayT = std::array<std::array<int16_t, Board::BoardFields>, Board::BoardsCount>;

    // for each color and each figure including lack of figure /pawn/knigh/bishop/rook/queen/
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

    static constexpr int16_t FullPhase = 4*KnightPhase + 4*BishopPhase + 4*RookPhase + 2*QueenPhase;
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
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        10, 10, 20, 25, 25, 10, 10, 10,
        5, 7, 10, 20, 20, 10, 7, 5,
        5, 5, 5, 0, 0, 5, 5, 5,
        5, 8, 8, -20, -20, 8, 8, 5,
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
        -50,-50,-50, -50, -50,-50,-50,-50,
        -30, -30, -30, -30, -30, -30, -30,-30,
        -20, -20, -20, -20, -20, -20, -20,-20,
         -15,  -15,  -15,  -15,  -15,  -15, -15, -15,
          -10,  -5,  -5,  -5,  -5,  -5,  -5, -10,
        -10,  5,  5,  5,  5,  5,  5,-10,
        -10,  15,  15,  15,  15,  15,  15,-10,
        -20,-10,-10, 30, 30,-10,-10,-20
    };

    static constexpr int16_t BasicBlackQueenEndPositionValues[]
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
          0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
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
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
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
                arr[BlackIndex + i][j] = -(BasicFigureValues[i] + BasicBlackPositionValues[i][ConvertToReversedPos(j)]);
        }

        constexpr size_t WhiteIndex = WHITE * Board::BoardsPerCol;
        for (size_t i = 0; i <= kingIndex; ++i)
        {
            for (size_t j = 0; j < Board::BoardFields; ++j)
                arr[WhiteIndex + i][j] = BasicFigureValues[i] + BasicBlackPositionValues[i][j];
        }

        return arr;
    } ();

    static inline MaterialArrayT _materialTable = []
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

            // Applying Knight pair penalty -> Knighs are losing value when less pawns are on board
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
};

#endif  // BOARDEVALUATOR_H
