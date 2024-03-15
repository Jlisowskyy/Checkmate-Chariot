//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include <climits>
#include <format>
#include <string>
#include <vector>

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/MoveGenerator.h"
#include "TranspositionTable.h"
#include "ZobristHash.h"

struct BestMoveSearch
{
    // ------------------------------
    // Class creation
    // ------------------------------

    BestMoveSearch() = delete;
    BestMoveSearch(const Board& board, stack<Move, DefaultStackSize>& s) : _stack(s), _board(board) {}
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <class FullBoardEvalFuncT, bool WriteInfo = true>
    void IterativeDeepening(FullBoardEvalFuncT evalF, Move* output, const uint8_t maxDepth)
    {
        MoveGenerator generator(_board, _stack);
        const auto moves = generator.GetMovesFast();
        std::vector<std::pair<int, int>> sortedMoveList(moves.size + 1);

        // sentinel guarded
        for (size_t i = 1; i <= moves.size; ++i)
            sortedMoveList[i] = std::make_pair(i - 1, 0);
        // sentinel
        sortedMoveList[0] = std::make_pair(-1, PositiveInfinity);

        uint64_t zHash = ZHasher.GenerateHash(_board);

        uint8_t depth = 1;
        int alpha = NegativeInfinity;
        while (alpha != PositiveInfinity && depth <= maxDepth - 1)
        {
            // resetting alpha
            alpha = NegativeInfinity;

            // saving old params
            const auto oldCastlings = _board.Castlings;
            const auto oldElPassant = _board.elPassantField;

            // list iteration
            int eval{};
            for (size_t i = 1; i <= moves.size; ++i)
            {
                Move::MakeMove(moves[sortedMoveList[i].first], _board);
                zHash = ZHasher.UpdateHash(zHash, moves[sortedMoveList[i].first], oldElPassant, oldCastlings);
                eval = -_alphaBeta(evalF, _board, NegativeInfinity, -alpha, depth, zHash);
                zHash = ZHasher.UpdateHash(zHash, moves[sortedMoveList[i].first], oldElPassant, oldCastlings);
                Move::UnmakeMove(moves[sortedMoveList[i].first], _board, oldCastlings, oldElPassant);

                alpha = std::max(alpha, eval);
                sortedMoveList[i].second = eval;
            }

            // move sorting
            _insertionSort(sortedMoveList, moves.size);
            *output = moves[sortedMoveList[1].first];

            if constexpr (WriteInfo)
            {
                GlobalLogger.StartLogging() << std::format("[ INFO ] Depth: {}, best move: {}, with eval: {}\n", depth + 1,
                                                           moves[sortedMoveList[1].first].GetLongAlgebraicNotation(), (double)eval/1000.0);
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
    template <class FullBoardEvalFuncT>
    [[nodiscard]] int _alphaBeta(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta, const uint8_t depthLeft, uint64_t zHash)
    {
        Move bestMove;
        Move prevMove;

        // last depth static eval needed or prev pv node value
        if (depthLeft == 0)
        {
            return _alphaBetaCaptures(evalF, bd, alpha, beta, zHash);
        }

        // saving old parameters for later usage
        const auto oldCastlings = bd.Castlings;
        const auto oldElPassant = bd.elPassantField;

        // reading Transposition table for previous score
        const auto prevSearchRes = TTable.GetRecord(zHash);

        // We got a hit -> process old results
        if (prevSearchRes.GetHash() == zHash)
        {
            // TODO: if previous results is a pv-node and depth of search
            if (prevSearchRes.IsPvNode() == true && prevSearchRes.GetDepth() >= depthLeft)
                return prevSearchRes.GetEval();

            // reading previous move
            prevMove = prevSearchRes.GetMove();

            // processing move
            Move::MakeMove(prevMove, bd);
            zHash = ZHasher.UpdateHash(zHash, prevMove, oldElPassant, oldCastlings);
            const int moveValue = -_alphaBeta(evalF, bd, -beta, -alpha, depthLeft - 1, zHash);
            zHash = ZHasher.UpdateHash(zHash, prevMove, oldElPassant, oldCastlings);
            Move::UnmakeMove(prevMove, bd, oldCastlings, oldElPassant);

            // we got a cut-off
            if (moveValue >= beta)
            {
                // replacing old record when we have better depth
                if (depthLeft >= prevSearchRes.GetDepth())
                {
                    const TranspositionTable::HashRecord record{zHash, prevMove,
                        moveValue, prevSearchRes.GetStatVal(), depthLeft, TranspositionTable::HashRecord::NoPvNode };
                    TTable.Add(record);
                }

                return beta;
            }

            // updating alpha
            if (moveValue > alpha)
            {
                alpha = moveValue;
                bestMove = prevMove;
            }
        }

        // generate moves
        MoveGenerator mechanics(bd, _stack);
        const auto moves = mechanics.GetMovesFast();

        // signalize checks and draws
        if (moves.size == 0)
        {
            _stack.PopAggregate(moves);
            return mechanics.IsCheck() ? NegativeInfinity : 0;
        }

        // sorting moves with simple heuristics
        _heuristicSort(moves);

        // processsing each move
        for (size_t i = 0; i < moves.size; ++i)
        {
            // skipping move found in transposition table
            if (moves[i] == prevMove) continue;

            // processing move
            Move::MakeMove(moves[i], bd);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            const int moveValue = -_alphaBeta(evalF, bd, -beta, -alpha, depthLeft - 1, zHash);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

            // cut-off found
            if (moveValue >= beta)
            {
                // cleaning stack
                _stack.PopAggregate(moves);

                // if profitable add move to table
                if (depthLeft >= prevSearchRes.GetDepth())
                {
                    const int statVal = prevSearchRes.GetHash() == zHash ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
                    const TranspositionTable::HashRecord record{zHash, moves[i],
                        moveValue, statVal, depthLeft, TranspositionTable::HashRecord::NoPvNode };
                    TTable.Add(record);
                }

                return beta;
            }

            // updating alpha
            if (moveValue > alpha)
            {
                alpha = moveValue;
                bestMove = moves[i];
            }
        }

        //clean up
        _stack.PopAggregate(moves);

        // updating if profitable
        if (depthLeft >= prevSearchRes.GetDepth())
        {
            const int statVal = prevSearchRes.GetHash() == zHash ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
            const TranspositionTable::HashRecord record{zHash, bestMove,
                alpha, statVal, depthLeft, TranspositionTable::HashRecord::PvNode };
            TTable.Add(record);
        }

        return alpha;
    }

    template <class FullBoardEvalFuncT>
    [[nodiscard]] int _alphaBetaID(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta, const int depth, uint64_t zHash)
    {
        if (depth < 4)
            return _alphaBeta(evalF, bd, alpha, beta, depth, zHash);

        MoveGenerator generator(bd, _stack);
        const auto moves = generator.GetMovesFast();

        if (moves.size == 0)
        {
            _stack.PopAggregate(moves);
            return generator.IsCheck() ? NegativeInfinity : 0;
        }

        std::vector<std::pair<int, int>> sortedMoveList(moves.size + 1);

        // sentinel guarded
        for (size_t i = 1; i <= moves.size; ++i)
            sortedMoveList[i] = std::make_pair(i - 1, 0);
        // sentinel
        sortedMoveList[0] = std::make_pair(-1, PositiveInfinity);

        // saving old params
        const auto oldCastlings = _board.Castlings;
        const auto oldElPassant = _board.elPassantField;

        size_t cutOffIndex = moves.size;
        for (int dep = 1; dep < depth - 2; ++dep)
        {
            // resetting alpha
            int inAlph = alpha;

            // list iteration
            for (size_t i = 1; i <= moves.size; ++i)
            {
                Move::MakeMove(moves[sortedMoveList[i].first], _board);
                zHash = ZHasher.UpdateHash(zHash, moves[sortedMoveList[i].first], oldElPassant, oldCastlings);
                int eval = -_alphaBetaID(evalF, _board, -beta, -inAlph, dep);
                zHash = ZHasher.UpdateHash(zHash, moves[sortedMoveList[i].first], oldElPassant, oldCastlings);
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

        for (size_t i = 1; i <= moves.size; ++i)
        {
            Move::MakeMove(moves[sortedMoveList[i].first], _board);
            zHash = ZHasher.UpdateHash(zHash, moves[sortedMoveList[i].first], oldElPassant, oldCastlings);
            int moveValue = -_alphaBetaID(evalF, _board, -beta, -alpha, depth - 1);
            zHash = ZHasher.UpdateHash(zHash, moves[sortedMoveList[i].first], oldElPassant, oldCastlings);
            Move::UnmakeMove(moves[sortedMoveList[i].first], _board, oldCastlings, oldElPassant);

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

    template <class FullBoardEvalFuncT>
    [[nodiscard]] int _alphaBetaCaptures(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta, uint64_t zHash)
    {
        const auto prevSearchRes = TTable.GetRecord(zHash);

        // int eval;
        // if (zHash == prevSearchRes.GetHash())
        // {
        //     if (prevSearchRes.GetStatVal() != TranspositionTable::HashRecord::NoEval)
        //         eval = prevSearchRes.GetStatVal();
        //     else
        //         eval = evalF(bd, bd.movColor);
        // }
        // else eval = evalF(bd, bd.movColor);

        const int eval = evalF(bd, bd.movColor);

        if (eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);

        MoveGenerator mechanics(bd, _stack);
        const auto moves = mechanics.GetMovesFast<true>();

        const auto oldCastlings = bd.Castlings;
        const auto oldElPassant = bd.elPassantField;

        _heuristicSort(moves);
        for (size_t i = 0; i < moves.size; ++i)
        {
            Move::MakeMove(moves[i], bd);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            const int moveValue = -_alphaBetaCaptures(evalF, bd, -beta, -alpha, zHash);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
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
    static void _heuristicSort(MoveGenerator::payload moves);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int NegativeInfinity = INT_MIN + 1;
    static constexpr int PositiveInfinity = INT_MAX;

    stack<Move, DefaultStackSize>& _stack;
    Board _board;
};

#endif  // BESTMOVESEARCH_H
