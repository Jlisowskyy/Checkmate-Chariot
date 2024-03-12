//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include <climits>
#include <string>
#include <vector>
#include <format>

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/MoveGenerator.h"

struct BestMoveSearch
{
    // ------------------------------
    // Class creation
    // ------------------------------

    BestMoveSearch() = delete;
    BestMoveSearch(const Board& board, stack<Move, DefaultStackSize>& s): _stack(s), _board(board) {}
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

template<
    class FullBoardEvalFuncT,
    bool WriteInfo = true
>void IterativeDeepening(FullBoardEvalFuncT evalF, Move* output, const int maxDepth)
    {
        MoveGenerator generator(_board, _stack);
        const auto moves = generator.GetMovesFast();
        std::vector<std::pair<int, int>> sortedMoveList(moves.size + 1);

        // sentinel guarded
        for (size_t i = 1 ; i <= moves.size; ++i)
            sortedMoveList[i] = std::make_pair(i-1, 0);
        //sentinel
        sortedMoveList[0] = std::make_pair(-1, PositiveInfinity);

        int depth = 1;
        int alpha = NegativeInfinity;
        while(alpha != PositiveInfinity && depth <= maxDepth-1){
            // resetting alpha
            alpha = NegativeInfinity;

            // saving old params
            const auto oldCastlings = _board.Castlings;
            const auto oldElPassant = _board.elPassantField;

            // list iteration
            for (size_t i = 1; i <= moves.size; ++i) {
                Move::MakeMove(moves[sortedMoveList[i].first], _board);
                int eval = -_alphaBetaID(evalF, _board, NegativeInfinity, -alpha, depth);
                Move::UnmakeMove(moves[sortedMoveList[i].first], _board, oldCastlings, oldElPassant);

                alpha = std::max(alpha, eval);
                sortedMoveList[i].second = eval;
            }

            // move sorting
            _insertionSort(sortedMoveList, moves.size);
            *output = moves[sortedMoveList[1].first];

            if constexpr (WriteInfo){
                GlobalLogger.StartLogging() << std::format("[ INFO ] Depth: {}, best move: {}\n",
                    depth+1, moves[sortedMoveList[1].first].GetLongAlgebraicNotation());
            }

            ++depth;
        }

        _stack.PopAggregate(moves);
    }

    // ------------------------------
    // Private class methods
    // ------------------------------
private:

    // ALPHA - minimum score of maximizing player
    // BETA - maximum score of minimazing player
    template<
        class FullBoardEvalFuncT
    > [[nodiscard]] int _alphaBeta(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta, const int depth)
    {
        if (depth == 0) {
            return _alphaBetaCaptures(evalF, bd, alpha, beta);
            // return evalF(bd, bd.movColor) - evalF(bd, SwapColor(bd.movColor));
        }

        MoveGenerator mechanics(bd, _stack);
        const auto moves = mechanics.GetMovesFast();

        if (moves.size == 0)
        {
            _stack.PopAggregate(moves);
            return mechanics.IsCheck() ? NegativeInfinity : 0;
        }

        const auto oldCastlings = bd.Castlings;
        const auto oldElPassant = bd.elPassantField;

        _heapSortMoves(moves);
        for(size_t i = 0; i < moves.size; ++i)
        {
            Move::MakeMove(moves[i], bd);
            const int moveValue = -_alphaBeta(evalF, bd, -beta, -alpha, depth-1);
            Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

            if (moveValue >= beta)
            {
                _stack.PopAggregate(moves);
                return beta;
            }

            alpha = std::max(alpha, moveValue);
        }

        _stack.PopAggregate(moves);
        return alpha;
    }

    template<
       class FullBoardEvalFuncT
   > [[nodiscard]] int _alphaBetaID(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta, const int depth)
    {
        if (depth < 4) return _alphaBeta(evalF, bd, alpha, beta, depth);

        MoveGenerator generator(bd, _stack);
        const auto moves = generator.GetMovesFast();
        std::vector<std::pair<int, int>> sortedMoveList(moves.size + 1);

        // sentinel guarded
        for (size_t i = 1 ; i <= moves.size; ++i)
            sortedMoveList[i] = std::make_pair(i-1, 0);
        //sentinel
        sortedMoveList[0] = std::make_pair(-1, PositiveInfinity);

        size_t cutOffIndex = moves.size;
        for (int dep = 1; dep < depth-1; ++dep){
            // resetting alpha
            int inAlph = alpha;

            // saving old params
            const auto oldCastlings = _board.Castlings;
            const auto oldElPassant = _board.elPassantField;

            // list iteration
            for (size_t i = 1; i <= moves.size; ++i) {
                Move::MakeMove(moves[sortedMoveList[i].first], _board);
                int eval = -_alphaBetaID(evalF, _board, -beta, -inAlph, dep);
                Move::UnmakeMove(moves[sortedMoveList[i].first], _board, oldCastlings, oldElPassant);

                if (eval >= beta)
                {
                    sortedMoveList[i].second = beta;
                    cutOffIndex = i;
                    break;
                }

                inAlph = std::max(inAlph, eval);
                sortedMoveList[i].second = eval;
            }

            // move sorting
            _insertionSort(sortedMoveList, cutOffIndex);
            cutOffIndex = moves.size;
        }

        _stack.PopAggregate(moves);
        return sortedMoveList[1].second;
    }

    template<
        class FullBoardEvalFuncT
    > [[nodiscard]] int _alphaBetaCaptures(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta)
    {
        const int eval = evalF(bd, bd.movColor);

        if (eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);

        MoveGenerator mechanics(bd, _stack);
        const auto moves = mechanics.GetMovesFast<true>();

        const auto oldCastlings = bd.Castlings;
        const auto oldElPassant = bd.elPassantField;

        _heapSortMoves(moves);
        for(size_t i = 0; i < moves.size; ++i)
        {
            Move::MakeMove(moves[i], bd);
            const int moveValue = -_alphaBetaCaptures(evalF, bd, -beta, -alpha);
            Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

            if (moveValue >= beta)
            {
                _stack.PopAggregate(moves);
                return beta;
            }

            alpha = std::max(alpha, moveValue);
        }

        _stack.PopAggregate(moves);
        return alpha;
    }

    static void _insertionSort(std::vector<std::pair<int, int>>& list, size_t range);
    static void _heapSortMoves(MoveGenerator::payload moves);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int NegativeInfinity = INT_MIN + 1;
    static constexpr int PositiveInfinity = INT_MAX;

    stack<Move, DefaultStackSize>& _stack;
    Board _board;
};

#endif //BESTMOVESEARCH_H
