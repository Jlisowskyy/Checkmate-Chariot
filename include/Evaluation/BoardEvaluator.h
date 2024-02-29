//
// Created by Jlisowskyy on 2/28/24.
//

#ifndef BOARDEVALUATOR_H
#define BOARDEVALUATOR_H

#include "../EngineTypeDefs.h"

/*      Collection of evaluation function
 *
 *  Additional notes:
 *  - currently only full board evalution is possible
 *  - incremental evaluation will be available when when move generation function will be fully avaialable
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

    // function uses only material to evaluate passed board
    [[nodiscard]] static int32_t NaiveEvaluation1(const Board& bd, const int color)
    {
        const size_t allyIndex = color*Board::BoardsPerCol;
        int materialValue = 0;

        for (size_t i = 0; i < kingIndex; ++i)
            for (uint64_t figs = bd.boards[allyIndex + i]; figs; figs ^= ExtractLsbBit(figs))
                materialValue += BasicFigureValues[i];

        return materialValue;
    }

    [[nodiscard]] static int32_t NaiveEvaluation2(const Board& bd, const int color)
    {
        const size_t allyIndex = color*Board::BoardsPerCol;
        int materialValue = 0;

        // iterate through boards
        for (size_t i = 0; i < kingIndex; ++i)
        {
            // extract figures board
            uint64_t figs = bd.boards[allyIndex + i];

            // iterate through figures
            while(figs)
            {
                const int figPos = ExtractMsbPos(figs);

                // sum costs offseted by position
                materialValue += CostsWithPositionsIncluded[allyIndex + i][figPos];

                // remove processed figures
                figs ^= maxMsbPossible >> figPos;
            }
        }

        return materialValue;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------
private:

    // ------------------------------
    // Class fields
    // ------------------------------
public:
    using CostArrayT = std::array<std::array<int32_t, Board::BoardFields>, Board::BoardsCount>;

    static constexpr int32_t BasicFigureValues[]
    {
        1000, // Pawn
        3300, // Knight
        3500, // Bishop
        5000, // Rook
        9000, // Queen
    };

    static constexpr int32_t BasicBlackPawnPositionValues[]
    {
          0,   0,   0,    0,    0,    0,   0,   0,
        500, 500, 500,  500,  500,  500, 500, 500,
        100, 100, 200,  300,  300,  200, 100, 100,
         50,  50, 100,  250,  250,  100,  50,  50,
          0,   0,   0,  200,  200,    0,   0,   0,
         50, -50, -10,    0,    0, -100, -50,  50,
         50, 100, 100, -200, -200,  100, 100,  50,
          0,   0,   0,    0,    0,    0,   0,   0
    };

    static constexpr int32_t BasicBlackKnightPositionValues[]
    {
        -500, -400, -300, -300, -300, -300, -400, -500,
        -400, -200,    0,    0,    0,    0, -200, -400,
        -300,    0,  100,  150,  150,  100,    0, -300,
        -300,   50,  150,  200,  200,  150,   50, -300,
        -300,    0,  150,  200,  200,  150,    0, -300,
        -300,   50,  100,  150,  150,  100,   50, -300,
        -400, -200,    0,   50,   50,    0, -200, -400,
        -500, -400, -300, -300, -300, -300, -400, -500,
    };

    static constexpr int32_t BasicBlackBishopPositionValues[]
    {
        -200, -100, -100, -100, -100, -100, -100, -200,
        -100,    0,    0,    0,    0,    0,    0, -100,
        -100,    0,   50,  100,  100,   50,    0, -100,
        -100,   50,   50,  100,  100,   50,   50, -100,
        -100,    0,  100,  100,  100,  100,    0, -100,
        -100,  100,  100,  100,  100,  100,  100, -100,
        -100,   50,    0,    0,    0,    0,   50, -100,
        -200, -100, -100, -100, -100, -100, -100, -200,
    };

    static constexpr int32_t BasicBlackRookPositionValues[]
    {
          0,   0,   0,   0,   0,   0,   0,   0,
         50, 100, 100, 100, 100, 100, 100,  50,
        -50,   0,   0,   0,   0,   0,   0, -50,
        -50,   0,   0,   0,   0,   0,   0, -50,
        -50,   0,   0,   0,   0,   0,   0, -50,
        -50,   0,   0,   0,   0,   0,   0, -50,
        -50,   0,   0,   0,   0,   0,   0, -50,
          0,   0,   0,  50,  50,   0,   0,   0
    };

    static constexpr int32_t BasicBlackQueenPositionValues[]
    {
        -200, -100, -100, -50, -50, -100, -100, -200,
        -100,    0,    0,   0,   0,    0,    0, -100,
        -100,    0,   50,  50,  50,   50,    0, -100,
         -50,    0,   50,  50,  50,   50,    0,  -50,
           0,    0,   50,  50,  50,   50,    0,  -50,
        -100,   50,   50,  50,  50,   50,    0, -100,
        -100,    0,   50,   0,   0,    0,    0, -100,
        -200, -100, -100, -50, -50, -100, -100, -200
    };

    static constexpr const int32_t* BasicBlackPositionValues[]
    {
        BasicBlackPawnPositionValues,
        BasicBlackKnightPositionValues,
        BasicBlackBishopPositionValues,
        BasicBlackRookPositionValues,
        BasicBlackQueenPositionValues,
    };

    static constexpr CostArrayT CostsWithPositionsIncluded = []()
    {
        CostArrayT arr{};

        constexpr size_t BlackIndex = BLACK*Board::BoardsPerCol;
        for (size_t i = 0; i < kingIndex; ++i)
        {
            for (size_t j = 0; j < Board::BoardFields; ++j)
                arr[BlackIndex + i][j] = BasicFigureValues[i] + BasicBlackPositionValues[i][j];
        }

        constexpr size_t WhiteIndex = WHITE*Board::BoardsPerCol;
        for (size_t i = 0; i < kingIndex; ++i)
        {
            for (int j = 0; j < Board::BoardFields; ++j)
                arr[WhiteIndex + i][j] = BasicFigureValues[i] + BasicBlackPositionValues[i][ConvertToReversedPos(j)];
        }

        return arr;
    }();
};

#endif //BOARDEVALUATOR_H
