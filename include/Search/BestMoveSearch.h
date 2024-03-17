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
        auto moves = generator.GetMovesFast();

        // saving old params
        const auto oldCastlings = _board.Castlings;
        const auto oldElPassant = _board.elPassantField;

        uint64_t zHash = ZHasher.GenerateHash(_board);
        for (int depth = 0; depth < maxDepth; ++depth)
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

                GlobalLogger.StartLogging() << std::format("info depth: {}, best move: {}, eval: {}, time: {}, nodes: {}, nodes per sec: {}, tt entries: {}, at age: {}\n", depth + 1,
                                                           moves[0].GetLongAlgebraicNotation(), static_cast<double>(moves[0].GetEval())/100.0,
                                                           spentMs, _visitedNodes, nps, TTable.GetContainedElements(), _age);
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
    [[nodiscard]] int _alphaBeta(FullBoardEvalFuncT evalF, Board& bd, int alpha, int beta, const int depthLeft, uint64_t zHash)
    {
        const int alphaStart = alpha;
        Move bestMove;
        int bestEval = NegativeInfinity;
        bool wasTTHit = false;

        // incrementing nodes counter;
        ++_visitedNodes;

        // reading Transposition table for previous score
        const auto prevSearchRes = TTable.GetRecord(zHash);

        // We got a hit
        if (prevSearchRes.GetHash() == zHash)
        {
            wasTTHit = true;

            if (prevSearchRes.GetDepth() >= depthLeft)
            {
                switch (prevSearchRes.GetNodeType())
                {
                    case pvNode:
                        return prevSearchRes.GetEval();
                    case lowerBound:
                        alpha = std::max(alpha, prevSearchRes.GetEval());
                        break;
                    case upperBound:
                        beta = std::min(beta, prevSearchRes.GetEval());
                        break;
                    default:
                        break;
                }

                if (alpha > beta)
                    return prevSearchRes.GetEval();
            }
        }

        // last depth static eval needed or prev pv node value
        if (depthLeft == 0)
            return _alphaBetaCaptures(evalF, bd, alpha, beta, zHash);

        // generate moves
        MoveGenerator mechanics(bd, _stack);
        auto moves = mechanics.GetMovesFast();

        // signalize checks and draws
        if (moves.size == 0)
        {
            _stack.PopAggregate(moves);
            return mechanics.IsCheck() ? NegativeInfinity : 0;
        }

        // sorting moves with simple heuristics
        _embeddedMoveSort(moves, moves.size);

        // saving old params
        const auto oldCastlings = bd.Castlings;
        const auto oldElPassant = bd.elPassantField;

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
        if (depthLeft >= prevSearchRes.GetDepth() || (wasTTHit == false && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
        {
            const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
            const nodeType nType = bestEval <= alphaStart ? upperBound :
                                    bestEval >= beta      ? lowerBound :
                                                            pvNode;


            const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statVal, depthLeft, nType, _age };
            TTable.Add(record);
        }

        return alpha;
    }

    template <class FullBoardEvalFuncT>
    [[nodiscard]] int _alphaBetaID(FullBoardEvalFuncT evalF, Board& bd, int alpha, int beta, int depthLeft, uint64_t zHash);

    template <class FullBoardEvalFuncT>
    [[nodiscard]] int _alphaBetaCaptures(FullBoardEvalFuncT evalF, Board& bd, int alpha, int beta, uint64_t zHash)
    {
        const int alphaStart = alpha;
        Move bestMove;
        int bestEval = NegativeInfinity;
        bool wasTTHit = false;
        ++_visitedNodes;

        // reading record from tt mape
        const auto prevSearchRes = TTable.GetRecord(zHash);

        int eval;
        if (prevSearchRes.GetHash() == zHash)
        {
            wasTTHit = true;

            switch (prevSearchRes.GetNodeType())
            {
                case pvNode:
                    return prevSearchRes.GetEval();
                case lowerBound:
                    alpha = std::max(alpha, prevSearchRes.GetEval());
                break;
                case upperBound:
                    beta = std::min(beta, prevSearchRes.GetEval());
                break;
                default:
                    break;
            }

            if (alpha > beta)
                return prevSearchRes.GetEval();

            if (prevSearchRes.GetStatVal() != TranspositionTable::HashRecord::NoEval)
                eval = prevSearchRes.GetStatVal();
            else eval = evalF(bd, bd.movColor);
        }
        else eval = evalF(bd, bd.movColor);

        if (eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);

        // generating moves
        MoveGenerator mechanics(bd, _stack);
        auto moves = mechanics.GetMovesFast<true>();

        // end of path
        if (moves.size == 0)
            return eval;

        // sorting moves
        _embeddedMoveSort(moves, moves.size);

        // saving old params
        const auto oldCastlings = bd.Castlings;
        const auto oldElPassant = bd.elPassantField;

        // processing best move first
        if (wasTTHit && prevSearchRes.GetMove().IsAttackingMove())
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
        if (prevSearchRes.IsEmpty() || (wasTTHit == false && _age - prevSearchRes.GetAge() >= QuisenceAgeDiffToReplace))
        {
            const nodeType nType = bestEval <= alphaStart ? upperBound :
                                    bestEval >= beta      ? lowerBound :
                                                            shallowNode;

            const TranspositionTable::HashRecord record{zHash, bestMove,
               bestEval, eval, 0, nType, _age };

            TTable.Add(record);
        }

        return alpha;
    }

    static void _embeddedMoveSort(MoveGenerator::payload moves, size_t range);
    static void _pullMoveToFront(MoveGenerator::payload moves, Move mv);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int NegativeInfinity = INT16_MIN + 100;
    static constexpr int PositiveInfinity = INT16_MAX - 100;

    static constexpr uint16_t QuisenceAgeDiffToReplace = 16;
    static constexpr uint16_t SearchAgeDiffToReplace = 8;

    stack<Move, DefaultStackSize>& _stack;
    Board _board;
    const uint16_t _age;
    uint64_t _visitedNodes = 0;
};

template<class FullBoardEvalFuncT>
int BestMoveSearch::_alphaBetaID(FullBoardEvalFuncT evalF, Board& bd, int alpha, int beta, const int depthLeft,
    uint64_t zHash)
{
    if (depthLeft < 3)
        return _alphaBeta(evalF, bd, alpha, beta, depthLeft, zHash);

    const int alphaStart = alpha;
    Move bestMove;
    int bestEval = NegativeInfinity;
    bool wasTTHit = false;

    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    if (prevSearchRes.GetHash() == zHash)
    {
        wasTTHit = true;

        if (prevSearchRes.GetDepth() >= depthLeft)
        {
            switch (prevSearchRes.GetNodeType())
            {
                case pvNode:
                    return prevSearchRes.GetEval();
                case lowerBound:
                    alpha = std::max(alpha, prevSearchRes.GetEval());
                break;
                case upperBound:
                    beta = std::min(beta, prevSearchRes.GetEval());
                break;
                default:
                    break;
            }

            if (alpha > beta)
                return prevSearchRes.GetEval();
        }
    }

    // generating moves
    MoveGenerator generator(bd, _stack);
    auto moves = generator.GetMovesFast();

    if (moves.size == 0)
    {
        _stack.PopAggregate(moves);
        return generator.IsCheck() ? NegativeInfinity : 0;
    }

    size_t cutOffIndex = moves.size;
    _embeddedMoveSort(moves, cutOffIndex);

    // saving old params
    const auto oldCastlings = _board.Castlings;
    const auto oldElPassant = _board.elPassantField;

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

    if (depthLeft >= prevSearchRes.GetDepth() || (wasTTHit == false && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
        const nodeType nType = bestEval <= alphaStart ? upperBound :
                                bestEval >= beta      ? lowerBound :
                                                        pvNode;


        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statVal, depthLeft, nType, _age };
        TTable.Add(record);
    }

    return alpha;
}

#endif  // BESTMOVESEARCH_H
