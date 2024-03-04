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
// #include "../MoveGeneration/ChessMechanics.h"
#include "../MoveGeneration/MoveGenerator.h"

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

    template<
        class FullBoardEvalFuncT,
        bool WriteInfo = false
    > [[nodiscard]] std::string searchMoveTimeFullBoardEvalUnthreaded(FullBoardEvalFuncT evalF, long timeLimit)
    {
        auto timeStart = std::chrono::steady_clock::now();

        MoveGenerator mechanics(_board);
        auto moves = mechanics.GetMovesFast();
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
            _insertionSort(sortedMoveList);

            // saving result
            bestMove = GetShortAlgebraicMoveEncoding(_board, moves[sortedMoveList[1].first]);

            if constexpr (WriteInfo){
                GlobalLogger.StartLogging() << std::format("[ INFO ] Depth: {}, best move: {}\n", depth, bestMove);
            }

            ++depth;
        }
    }

    template<
        class FullBoardEvalFuncT,
        bool WriteInfo = false
    > [[nodiscard]] std::string searchMoveDepthFullBoardEvalUnthreaded(FullBoardEvalFuncT evalF, const int targetDepth)
    {
        MoveGenerator mechanics(_board);
        auto moves = mechanics.GetMovesFast();
        std::vector<std::pair<int, int>> sortedMoveList(moves.size() + 1);

        // sentinel guarded
        for (size_t i = 1 ; i <= moves.size(); ++i)
            sortedMoveList[i] = std::make_pair(i-1, 0);
        //sentinel
        sortedMoveList[0] = std::make_pair(-1, PositiveInfinity);

        for(int depth = 1; depth <= targetDepth; ++depth){
            // resetting alpha
            int alpha = NegativeInfinity;

            // list iteration
            for (size_t i = 1; i <= moves.size(); ++i) {
                int eval = -_alphaBeta(evalF, moves[sortedMoveList[i].first], NegativeInfinity, -alpha, depth);
                alpha = std::max(alpha, eval);
                sortedMoveList[i].second = eval;
            }

            // move sorting
            _insertionSort(sortedMoveList);

            if constexpr (WriteInfo){
                auto bestMove = GetShortAlgebraicMoveEncoding(_board, moves[sortedMoveList[1].first]);
                GlobalLogger.StartLogging() << std::format("[ INFO ] Depth: {}, best move: {}\n", depth, bestMove);
            }
        }

        return GetShortAlgebraicMoveEncoding(_board, moves[sortedMoveList[1].first]);
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
            return evalF(bd, bd.movColor) - evalF(bd, SwapColor(bd.movColor));
        }

        MoveGenerator mechanics(bd);
        auto moves = mechanics.GetMovesFast();

        if (moves.empty())
            return mechanics.IsCheck() ? NegativeInfinity : 0;

        for (auto& moveBoard : moves)
        {
            const int moveValue = -_alphaBeta(evalF, moveBoard, -beta, -alpha, depth-1);

            if (moveValue >= beta)
                return beta;

            alpha = std::max(alpha, moveValue);
        }

        return alpha;
    }

    static void _insertionSort(std::vector<std::pair<int, int>>& list);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int NegativeInfinity = INT_MIN + 1;
    static constexpr int PositiveInfinity = INT_MAX;

    Board _board;
};

#endif //BESTMOVESEARCH_H
