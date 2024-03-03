//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include <climits>
#include <string>
#include <chrono>
#include <vector>
#include <format>

#include "../EngineTypeDefs.h"
#include "../Evaluation/BoardEvaluator.h"
#include "../Interface/FenTranslator.h"
#include "../MoveGeneration/ChessMechanics.h"

struct BestMoveSearch
{
    // ------------------------------
    // Class creation
    // ------------------------------

    BestMoveSearch() = delete;
    BestMoveSearch(const Board& board): _board(board) {}
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------


    // TODO: add checks and draws
    template<
        class FullBoardEvalFuncT,
        bool WriteInfo = false
    > [[nodiscard]] std::string searchMoveTimeFullBoardEvalUnthreaded(FullBoardEvalFuncT evalF, long timeLimit)
    {
        auto timeStart = std::chrono::steady_clock::now();

        ChessMechanics mechanics(_board);
        auto moves = mechanics.GetPossibleMoveSlow();
        std::vector<std::pair<int, int>> sortedMoveList(moves.size() + 1);

        // sentinel guarded
        for (size_t i = 1 ; i <= moves.size(); ++i)
            sortedMoveList[i] = std::make_pair(i-1, 0);
        //sentinel
        sortedMoveList[0] = std::make_pair(-1, PositiveInfinity);

        int depth = 1;
        std::string bestMove;
        while(true){
            // resetting alpha
            int alpha = NegativeInfinity;

            // list iteration
            for (size_t i = 1; i <= moves.size(); ++i) {
                // comparing spent time
                auto timeStop = std::chrono::steady_clock::now();
                auto timeSpent = std::chrono::duration_cast<std::chrono::milliseconds>(timeStop - timeStart);

                if (timeSpent.count() > timeLimit)
                    return bestMove;

                int eval = -_alphaBeta(evalF, moves[sortedMoveList[i].first], NegativeInfinity, -alpha, depth);

                alpha = std::max(alpha, eval);
                sortedMoveList[i].second = eval;
            }

            // move sorting
            InsertionSort(sortedMoveList);

            // saving result
            bestMove = GetShortAlgebraicMoveEncoding(_board, moves[sortedMoveList[1].first]);

            if constexpr (WriteInfo){
                GlobalLogger.StartLogging() << std::format("[ INFO ] Depth: {}, best move: {}\n", depth, bestMove);
            }

            ++depth;
        }

        return bestMove;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------
private:

    // ALPHA - minimum score of maximizing player
    // BETA - maximum score of minimazing player
    template<
        class FullBoardEvalFuncT
    > [[nodiscard]] int _alphaBeta(FullBoardEvalFuncT evalF, Board& bd, int alpha, int beta, const int depth)
    {
        if (depth == 0) {
            int evaluation = evalF(bd, bd.movColor) - evalF(bd, SwapColor(bd.movColor));

            return evaluation;
        }

        ChessMechanics mechanics(bd);
        auto moves = mechanics.GetPossibleMoveSlow();

        // TODO: CHECKS FOR DRAW AND CHECK

        for (auto& moveBoard : moves)
        {
            const int moveValue = -_alphaBeta(evalF, moveBoard, -beta, -alpha, depth-1);

            if (moveValue >= beta)
                return beta;

            alpha = std::max(alpha, moveValue);
        }

        return alpha;
    }

    static void InsertionSort(std::vector<std::pair<int, int>>& list){
        for (size_t i = 2; i < list.size(); ++i)
        {
            size_t j = i - 1;
            auto val = list[i];
            while(list[j].second < val.second)
            {
                list[j+1] = list[j];
                j--;
            }
            list[j+1] = val;
        }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int NegativeInfinity = INT_MIN + 1;
    static constexpr int PositiveInfinity = INT_MAX;

    Board _board;
};

#endif //BESTMOVESEARCH_H
