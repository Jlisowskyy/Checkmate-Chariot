//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include <climits>
#include <format>
#include <string>
#include <vector>
#include <chrono>

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
    BestMoveSearch(const Board& board, stack<Move, DefaultStackSize>& s, const uint16_t age) : _stack(s), _board(board), _age(age) {}
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <class FullBoardEvalFuncT, bool WriteInfo = true>
    void IterativeDeepening(FullBoardEvalFuncT evalF, Move* output, const int maxDepth)
    {
        MoveGenerator generator(_board, _stack);
        const auto moves = generator.GetMovesFast();

        // saving old params
        const auto oldCastlings = _board.Castlings;
        const auto oldElPassant = _board.elPassantField;

        uint64_t zHash = ZHasher.GenerateHash(_board);
        for (int depth = 1; depth < maxDepth; ++depth)
        {
            // measuring time
            [[maybe_unused]]auto t1 = std::chrono::steady_clock::now();

            // move list iteration
            _visitedNodes = 0;
            int eval{};
            for (size_t i = 0; i < moves.size; ++i)
            {
                Move::MakeMove(moves[i], _board);
                zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
                eval = -_alphaBeta(evalF, _board, NegativeInfinity, PositiveInfinity, depth, zHash);
                zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
                Move::UnmakeMove(moves[i], _board, oldCastlings, oldElPassant);

                moves[i].ReplaceEval(static_cast<int16_t>(eval));
            }

            // measurment end
            [[maybe_unused]]auto t2 = std::chrono::steady_clock::now();

            // move sorting
            _embeddedMoveSort(moves, moves.size);
            *output = moves[0];

            if constexpr (WriteInfo)
            {
                static constexpr uint64_t MSEC = 1000 * 1000; // in nsecs
                const uint64_t spentMs = std::max(1LU, (t2-t1).count()/MSEC);
                const uint64_t nps = 1000LLU * _visitedNodes / spentMs;

                GlobalLogger.StartLogging() << std::format("info depth: {}, best move: {}, eval: {}, time: {}, nodes: {}, nodes per sec: {}, tt entries: {}\n", depth + 1,
                                                           moves[0].GetLongAlgebraicNotation(), static_cast<double>(eval)/100.0,
                                                           spentMs, _visitedNodes, nps, TTable.GetContainedElements());
            }
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
    [[nodiscard]] int _alphaBeta(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta, const int depthLeft, uint64_t zHash)
    {
        Move bestMove;
        int bestEval = NegativeInfinity;
        bool wasTTHit = false;

        // incrementing nodes counter;
        ++_visitedNodes;

        // last depth static eval needed or prev pv node value
        if (depthLeft == 0)
            return _alphaBetaCaptures(evalF, bd, alpha, beta, zHash);

        // saving old parameters for later usage
        const auto oldCastlings = bd.Castlings;
        const auto oldElPassant = bd.elPassantField;

        // reading Transposition table for previous score
        const auto prevSearchRes = TTable.GetRecord(zHash);

        // We got a hit
        if (prevSearchRes.GetHash() == zHash)
            wasTTHit = true;

        // generate moves
        MoveGenerator mechanics(bd, _stack);
        const auto moves = mechanics.GetMovesFast();

        // signalize checks and draws
        if (moves.size == 0)
        {
            _stack.PopAggregate(moves);
            return mechanics.IsCheck() == false ?  0 :
                           bd.movColor == WHITE ?  NegativeInfinity :
                                                   PositiveInfinity;
        }

        // sorting moves with simple heuristics
        _embeddedMoveSort(moves, moves.size);

        if (wasTTHit)
            _pullMoveToFront(moves, prevSearchRes.GetMove());

        // processsing each move
        for (size_t i = 0; i < moves.size; ++i)
        {
            // processing move
            Move::MakeMove(moves[i], bd);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            const int moveEval = -_alphaBeta(evalF, bd, -beta, -alpha, depthLeft - 1, zHash);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

            // cut-off found
            if (moveEval >= beta)
            {
                // cleaning stack
                _stack.PopAggregate(moves);

                // if profitable add move to table
                if (depthLeft >= prevSearchRes.GetDepth() || _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace)
                {
                    const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
                    const TranspositionTable::HashRecord record{zHash, moves[i],
                        moveEval, statVal, depthLeft, TranspositionTable::HashRecord::NoPvNode, _age };
                    TTable.Add(record);
                }

                return beta;
            }

            // updating alpha
            alpha = std::max(alpha, moveEval);

            if (moveEval > bestEval)
            {
                bestEval = moveEval;
                bestMove = moves[i];
            }
        }

        //clean up
        _stack.PopAggregate(moves);

        // updating if profitable
        if (depthLeft >= prevSearchRes.GetDepth() || _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace)
        {
            const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
            const TranspositionTable::HashRecord record{zHash, bestMove,
                bestEval, statVal, depthLeft, TranspositionTable::HashRecord::PvNode, _age };
            TTable.Add(record);
        }

        return alpha;
    }

    template <class FullBoardEvalFuncT>
    [[nodiscard]] int _alphaBetaID(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta, const int depthLeft, uint64_t zHash)
    {
        if (depthLeft < 3)
            return _alphaBeta(evalF, bd, alpha, beta, depthLeft, zHash);

        Move bestMove;
        int bestEval = NegativeInfinity;
        bool wasTTHit = false;

        // saving old params
        const auto oldCastlings = _board.Castlings;
        const auto oldElPassant = _board.elPassantField;

        const auto prevSearchRes = TTable.GetRecord(zHash);

        // We got a hit
        if (prevSearchRes.GetHash() == zHash)
            wasTTHit = true;

        // generating moves
        MoveGenerator generator(bd, _stack);
        const auto moves = generator.GetMovesFast();

        if (moves.size == 0)
        {
            _stack.PopAggregate(moves);
            return generator.IsCheck() == false ?  0 :
                           bd.movColor == WHITE ?  NegativeInfinity :
                                                   PositiveInfinity;
        }

        size_t cutOffIndex = moves.size;
        _embeddedMoveSort(moves, moves.size);
        for (int dep = 1; dep < depthLeft - 2; ++dep)
        {
            // resetting alpha
            int inAlph = alpha;

            if (wasTTHit && prevSearchRes.GetDepth() >= dep)
                _pullMoveToFront(moves, prevSearchRes.GetMove());

            // move list iteration
            for (size_t i = 0; i < moves.size; ++i)
            {
                Move::MakeMove(moves[i], _board);
                zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
                int eval = -_alphaBetaID(evalF, _board, -beta, -inAlph, dep, zHash);
                zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
                Move::UnmakeMove(moves[i], _board, oldCastlings, oldElPassant);

                if (eval >= beta)
                {
                    moves[i].ReplaceEval(static_cast<int16_t>(eval));
                    cutOffIndex = i;
                    break;
                }

                inAlph = std::max(inAlph, eval);
                moves[i].ReplaceEval(static_cast<int16_t>(eval));
            }

            // move sorting
            _embeddedMoveSort(moves, cutOffIndex);
            cutOffIndex = moves.size;
        }

        if (wasTTHit && prevSearchRes.GetDepth() >= depthLeft)
            _pullMoveToFront(moves, prevSearchRes.GetMove());

        // processing final depth
        for (size_t i = 0; i < moves.size; ++i)
        {
            Move::MakeMove(moves[i], _board);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            int moveValue = -_alphaBetaID(evalF, _board, -beta, -alpha, depthLeft - 1, zHash);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            Move::UnmakeMove(moves[i], _board, oldCastlings, oldElPassant);

            // cut-off found
            if (moveValue >= beta)
            {
                // cleaning up
                _stack.PopAggregate(moves);

                // if profitable add move to table
                if (depthLeft >= prevSearchRes.GetDepth() || _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace)
                {
                    const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
                    const TranspositionTable::HashRecord record{zHash, moves[i],
                        moveValue, statVal, depthLeft, TranspositionTable::HashRecord::NoPvNode, _age };
                    TTable.Add(record);
                }

                return beta;
            }

            alpha = std::max(alpha, moveValue);

            if (moveValue > bestEval)
            {
                bestEval = moveValue;
                bestMove = moves[i];
            }
        }

        _stack.PopAggregate(moves);

        if (depthLeft >= prevSearchRes.GetDepth() || _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace)
        {
            const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
            const TranspositionTable::HashRecord record{zHash, bestMove,
                bestEval, statVal, depthLeft, TranspositionTable::HashRecord::PvNode, _age };
            TTable.Add(record);
        }

        return alpha;
    }

    template <class FullBoardEvalFuncT>
    [[nodiscard]] int _alphaBetaCaptures(FullBoardEvalFuncT evalF, Board& bd, int alpha, const int beta, uint64_t zHash)
    {
        Move bestMove;
        int bestEval = NegativeInfinity;
        bool wasTTHit = false;
        ++_visitedNodes;

        // reading record from tt mape
        const auto prevSearchRes = TTable.GetRecord(zHash);

        // saving old params
        const auto oldCastlings = bd.Castlings;
        const auto oldElPassant = bd.elPassantField;

        // checking map record
        if (zHash == prevSearchRes.GetHash() && prevSearchRes.GetMove().IsAttackingMove())
            wasTTHit = true;

        const int eval = evalF(bd, bd.movColor);
        if (eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);

        // generating moves
        MoveGenerator mechanics(bd, _stack);
        const auto moves = mechanics.GetMovesFast<true>();

        // sorting moves
        _embeddedMoveSort(moves, moves.size);

        // processing best move first
        if (wasTTHit)
            _pullMoveToFront(moves, prevSearchRes.GetMove());

        // iterating through moves
        for (size_t i = 0; i < moves.size; ++i)
        {
            Move::MakeMove(moves[i], bd);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            const int moveValue = -_alphaBetaCaptures(evalF, bd, -beta, -alpha, zHash);
            zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
            Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

            if (moveValue >= beta)
            {
                // clean up
                _stack.PopAggregate(moves);

                // if profitable add move to table
                if (_age - prevSearchRes.GetAge() >= QuisenceAgeDiffToReplace)
                {
                    const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
                    const TranspositionTable::HashRecord record{zHash, moves[i],
                        moveValue, statVal, 0, TranspositionTable::HashRecord::NoPvNode, _age };
                    TTable.Add(record);
                }

                return beta;
            }

            if (moveValue > bestEval)
            {
                bestMove = moves[i];
                bestEval = moveValue;
            }

            alpha = std::max(alpha, moveValue);
        }

        // clean up
        _stack.PopAggregate(moves);

        // updating if profitable
        if (_age - prevSearchRes.GetAge() >= QuisenceAgeDiffToReplace)
        {
            const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
            const TranspositionTable::HashRecord record{zHash, bestMove,
                bestEval, statVal, 0, TranspositionTable::HashRecord::PvNode, _age };
            TTable.Add(record);
        }

        return alpha;
    }

    static void _embeddedMoveSort(MoveGenerator::payload moves, size_t range);
    static void _pullMoveToFront(MoveGenerator::payload moves, Move mv);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int NegativeInfinity = INT16_MIN + 1;
    static constexpr int PositiveInfinity = INT16_MAX;

    static constexpr uint16_t QuisenceAgeDiffToReplace = 16;
    static constexpr uint16_t SearchAgeDiffToReplace = 8;

    stack<Move, DefaultStackSize>& _stack;
    Board _board;
    const uint16_t _age;
    uint64_t _visitedNodes = 0;
};

#endif  // BESTMOVESEARCH_H
