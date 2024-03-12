//
// Created by Jlisowskyy on 2/28/24.
//

#ifndef BOARDEVALUATOR_H
#define BOARDEVALUATOR_H

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/BlackPawnMap.h"
#include "../MoveGeneration/KingMap.h"
#include "../MoveGeneration/WhitePawnMap.h"

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

    [[nodiscard]] static int32_t DefaultFullEvalFunction(const Board& bd, const int color)
    {
        const int whiteEval = NaiveEvaluation3(bd);
        return color == WHITE ? whiteEval : -whiteEval;
    }

    // function uses only material to evaluate passed board
    [[nodiscard]] static int32_t NaiveEvaluation1(const Board& bd)
    {
        int eval = 0;

        for (size_t i = 0; i < Board::BoardsCount; ++i)
            eval += CountOnesInBoard(bd.boards[i]) * BasicFigureValues[i];

        return eval;
    }

    [[nodiscard]] static int32_t NaiveEvaluation2(const Board& bd)
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

    [[nodiscard]] static int32_t NaiveEvaluation3(const Board& bd)
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

    // ------------------------------
    // Private class methods
    // ------------------------------
   private:
    static int32_t _applyBonusForCoveredPawns(const Board& bd, int32_t eval)
    {
        const uint64_t covereWPawns = WhitePawnMap::GetAttackFields(bd.boards[wPawnsIndex]) & bd.boards[wPawnsIndex];
        eval += CountOnesInBoard(covereWPawns) * CoveredWPawnBonus;

        const uint64_t coveredBPawns = BlackPawnMap::GetAttackFields(bd.boards[bPawnsIndex]) & bd.boards[bPawnsIndex];
        eval += CountOnesInBoard(coveredBPawns) * CoveredBPawnBonus;

        return eval;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
   public:
    using CostArrayT = std::array<std::array<int32_t, Board::BoardFields>, Board::BoardsCount>;

    static constexpr int32_t CoveredWPawnBonus = 50;
    static constexpr int32_t CoveredBPawnBonus = -50;
    static constexpr int32_t CoveredWKingBonus = 15;
    static constexpr int32_t CoveredBKingBonus = -15;

    static constexpr int32_t BasicFigureValues[]{
        1000,    // Pawn
        3300,    // Knight
        3500,    // Bishop
        5000,    // Rook
        9000,    // Queen
        100000,  // king
        -1000,   // Pawn
        -3300,   // Knight
        -3500,   // Bishop
        -5000,   // Rook
        -9000,   // Queen
        -100000  // king
    };

    static constexpr int32_t BasicBlackPawnPositionValues[]{
        0,   0,   0,   0,   0,   0,   0,   0,    500,  500, 500, 500, 500, 500, 500, 500, 100, 100, 200, 300, 300, 200,
        100, 100, 100, 100, 100, 250, 250, 100,  100,  100, 50,  75,  100, 200, 200, 100, 75,  50,  50,  50,  50,  0,
        0,   50,  50,  50,  75,  75,  75,  -200, -200, 75,  75,  75,  0,   0,   0,   0,   0,   0,   0,   0};

    static constexpr int32_t BasicBlackKnightPositionValues[]{
        -500, -400, -300, -300, -300, -300, -400, -500, -400, -200, 0,    0,    0,    0,    -200, -400,
        -300, 0,    100,  150,  150,  100,  0,    -300, -300, 50,   150,  200,  200,  150,  50,   -300,
        -300, 0,    150,  200,  200,  150,  0,    -300, -300, 50,   100,  150,  150,  100,  50,   -300,
        -400, -200, 0,    50,   50,   0,    -200, -400, -500, -400, -300, -300, -300, -300, -400, -500,
    };

    static constexpr int32_t BasicBlackBishopPositionValues[]{
        -200, -100, -100, -100, -100, -100, -100, -200, -100, 0,    0,    0,    0,    0,    0,    -100,
        -100, 0,    50,   100,  100,  50,   0,    -100, -100, 50,   50,   100,  100,  50,   50,   -100,
        -100, 0,    100,  100,  100,  100,  0,    -100, -100, 100,  100,  100,  100,  100,  100,  -100,
        -100, 50,   0,    0,    0,    0,    50,   -100, -200, -100, -100, -100, -100, -100, -100, -200,
    };

    static constexpr int32_t BasicBlackRookPositionValues[]{
        0, 0,   0,   0,   0,   0, 0, 0, 50, 100, 100, 100, 100, 100, 100, 50, -50, 0,   0,   0, 0, 0,
        0, -50, -50, 0,   0,   0, 0, 0, 0,  -50, -50, 0,   0,   0,   0,   0,  0,   -50, -50, 0, 0, 0,
        0, 0,   0,   -50, -50, 0, 0, 0, 0,  0,   0,   -50, 0,   0,   0,   50, 50,  10,  0,   0};

    static constexpr int32_t BasicBlackQueenPositionValues[]{
        -200, -100, -100, -50, -50, -100, -100, -200, -100, 0,    0,    0,  0,  0,    0,    -100,
        0,    25,   25,   25,  25,  25,   25,   0,    0,    25,   25,   50, 50, 25,   25,   0,
        0,    25,   50,   50,  50,  50,   25,   0,    0,    50,   50,   50, 50, 50,   50,   0,
        -100, 0,    50,   0,   0,   0,    0,    -100, -200, -100, -100, 1,  1,  -100, -100, -200};

    static constexpr int32_t BasicBlackKingPositionValues[]{
        -300, -400, -400, -500, -500, -400, -400, -300, -300, -400, -400, -500, -500, -400, -400, -300,
        -300, -400, -400, -500, -500, -400, -400, -300, -300, -400, -400, -500, -500, -400, -400, -300,
        -200, -300, -300, -400, -400, -300, -300, -200, -100, -200, -200, -200, -200, -200, -200, -100,
        200,  200,  0,    0,    0,    0,    200,  200,  200,  300,  100,  0,    0,    100,  300,  200};

    static constexpr const int32_t* BasicBlackPositionValues[]{
        BasicBlackPawnPositionValues, BasicBlackKnightPositionValues, BasicBlackBishopPositionValues,
        BasicBlackRookPositionValues, BasicBlackQueenPositionValues,  BasicBlackKingPositionValues,
    };

    static constexpr CostArrayT CostsWithPositionsIncluded = []()
    {
        CostArrayT arr{};

        constexpr size_t BlackIndex = BLACK * Board::BoardsPerCol;
        for (size_t i = 0; i <= kingIndex; ++i)
        {
            for (size_t j = 0; j < Board::BoardFields; ++j)
                arr[BlackIndex + i][j] = -(BasicFigureValues[i] + BasicBlackPositionValues[i][j]);
        }

        constexpr size_t WhiteIndex = WHITE * Board::BoardsPerCol;
        for (size_t i = 0; i <= kingIndex; ++i)
        {
            for (int j = 0; j < static_cast<int>(Board::BoardFields); ++j)
                arr[WhiteIndex + i][j] = BasicFigureValues[i] + BasicBlackPositionValues[i][ConvertToReversedPos(j)];
        }

        return arr;
    }();
};

#endif  // BOARDEVALUATOR_H
