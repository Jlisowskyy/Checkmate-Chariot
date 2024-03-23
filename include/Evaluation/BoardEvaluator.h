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

struct BoardEvaluator
{
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
        const int whiteEval = NaiveEvaluation3(bd);
        return color == WHITE ? whiteEval : -whiteEval;
    }

    // function uses only material to evaluate passed board
    [[nodiscard]] static int32_t PlainMaterialEvaluation(const Board&bd);

    [[nodiscard]] static int32_t NaiveEvaluation2(const Board&bd);

    [[nodiscard]] static int32_t NaiveEvaluation3(const Board&bd);

    [[nodiscard]] static int32_t Evaluation1(const Board&bd)
    {
        return 1;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

private:
    static int32_t _applyBonusForCoveredPawns(const Board&bd, int32_t eval);

    static size_t _getMaterialBoardIndex(const Board&bd);

    static size_t _getMaterialBoardIndex(size_t P, size_t N, size_t B, size_t R, size_t Q,
                                         size_t p, size_t n, size_t b, size_t r,
                                         size_t q);

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

    using MaterialArrayT = std::array<int16_t, MaterialTableSize>;

    static constexpr int16_t CoveredWPawnBonus = 5;
    static constexpr int16_t CoveredBPawnBonus = -5;

    static constexpr int16_t BasicFigureValues[]{
        100, // Pawn
        330, // Knight
        350, // Bishop
        500, // Rook
        900, // Queen
        10000, // king
        -100, // Pawn
        -330, // Knight
        -350, // Bishop
        -500, // Rook
        -900, // Queen
        -10000 // king
    };

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
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 3, 3, 3, 3, 0, -10,
        -5, 0, 3, 5, 5, 3, 0, -5,
        0, 3, 5, 5, 5, 5, 3, -5,
        -10, 5, 5, 5, 5, 5, 0, -10,
        -10, 0, 5, 0, 0, 0, 0, -10,
        -20, -10, -10, 1, 1, -10, -10, -20
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

    static constexpr const int16_t* BasicBlackPositionValues[]{
        BasicBlackPawnPositionValues, BasicBlackKnightPositionValues, BasicBlackBishopPositionValues,
        BasicBlackRookPositionValues, BasicBlackQueenPositionValues, BasicBlackKingPositionValues,
    };

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

    static constexpr MaterialArrayT _materialTable = []() constexpr
    {
        constexpr auto _reverseMaterialIndex = [](const size_t index) constexpr -> std::array<size_t , 10>
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
            auto figArr = _reverseMaterialIndex(i);


        }

        return arr;
    }();
};

#endif  // BOARDEVALUATOR_H
