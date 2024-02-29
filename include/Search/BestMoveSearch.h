//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include <limits.h>
#include <string>

#include "../EngineTypeDefs.h"
#include "../Evaluation/BoardEvaluator.h"
#include "../Interface/FenTranslator.h"
#include "../MoveGeneration/ChessMechanics.h"

struct BestMoveSearch
{
    // ------------------------------
    // Class creation
    // ------------------------------

    BestMoveSearch() = default;
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template<
        class FullBoardEvalFuncT
    > [[nodiscard]] std::string SearchMoveTimeFullBoardEval(FullBoardEvalFuncT evalF, long mSecs)
    {
        std::cout << _alphaBeta(evalF, FenTranslator::GetDefault(), INT_MIN, INT_MAX, 7);

        return "";
    }

    // ------------------------------
    // Private class methods
    // ------------------------------
private:

    template<
        class FullBoardEvalFuncT
    > [[nodiscard]] int _alphaBeta(FullBoardEvalFuncT evalF, Board bd, int alpha, int beta, const int depth)
    {
        if (depth == 0) return evalF(bd, bd.movColor);

        ChessMechanics mechanics(bd);

        for (const auto& moveBoard : mechanics.GetPossibleMoveSlow())
        {
            const int moveValue = _alphaBeta(evalF, moveBoard, alpha, beta, depth-1);

            if (moveValue >= beta)
                return beta;
            if (moveValue > alpha)
                alpha = moveValue;
        }

        return alpha;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

};

#endif //BESTMOVESEARCH_H
