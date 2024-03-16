//
// Created by Jlisowskyy on 2/28/24.
//

#ifndef BOARDEVALUATOR_H
#define BOARDEVALUATOR_H

#include <iomanip>

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

    void static DisplayCosts()
    {
        for (size_t bd = 0 ; bd < Board::BoardsCount; ++bd)
        {
            for (int pos = 0; pos < Board::BoardFields; ++pos)
            {
                if (pos % 8 == 0) std::cout << std::endl;

                std::cout << std::setw(6) << CostsWithPositionsIncluded[bd][pos] << " , ";
            }

            std::cout << std::endl;
        }
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

    static constexpr int32_t CoveredWPawnBonus = 5;
    static constexpr int32_t CoveredBPawnBonus = -5;

    static constexpr int32_t BasicFigureValues[]{
        100,    // Pawn
        330,    // Knight
        350,    // Bishop
        500,    // Rook
        900,    // Queen
        10000,  // king
        -100,   // Pawn
        -330,   // Knight
        -350,   // Bishop
        -500,   // Rook
        -900,   // Queen
        -10000  // king
    };

    static constexpr int32_t BasicBlackPawnPositionValues[]
    {
        0,  0,  0,  0,  0,  0,  0,  0,
       50, 50, 50, 50, 50, 50, 50, 50,
       10, 10, 20, 30, 30, 20, 10, 10,
       10, 10, 20, 25, 25, 10, 10, 10,
        5,  7, 10, 20, 20, 10,  7,  5,
        5,  5,  5,  0,  0,  5,  5,  5,
        5,  8,  8,-20,-20,  8,  8,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    static constexpr int32_t BasicBlackKnightPositionValues[]
    {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    };

    static constexpr int32_t BasicBlackBishopPositionValues[]
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    };

    static constexpr int32_t BasicBlackRookPositionValues[]
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  5,  5,  5,  5,  0,  0
    };

    static constexpr int32_t BasicBlackQueenPositionValues[]
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  3,  3,  3,  3,  0,-10,
         -5,  0,  3,  5,  5,  3,  0, -5,
          0,  3,  5,  5,  5,  5,  3, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, 1, 1,-10,-10,-20
    };

    static constexpr int32_t BasicBlackKingPositionValues[]{
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    };

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
    }();
};

#endif  // BOARDEVALUATOR_H
