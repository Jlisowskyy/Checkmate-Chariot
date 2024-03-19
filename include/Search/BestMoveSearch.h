//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include <format>
#include <string>
#include <vector>
#include <chrono>

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/MoveGenerator.h"
#include "TranspositionTable.h"
#include "ZobristHash.h"
#include "../Evaluation/CounterMoveTable.h"

struct BestMoveSearch
{
    // ------------------------------
    // Class creation
    // ------------------------------

    BestMoveSearch() = delete;
    BestMoveSearch(const Board& board, stack<Move, DefaultStackSize>& s, const uint16_t age) :
        _stack(s), _board(board), _age(age) {}
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <bool WriteInfo = true>
    void IterativeDeepening(Move* output, const int maxDepth)
    {
        MoveGenerator generator(_board, _stack, _kTable);
        auto moves = generator.GetMovesFast();

        // saving old params
        const auto oldCastlings = _board.Castlings;
        const auto oldElPassant = _board.elPassantField;

        uint64_t zHash = ZHasher.GenerateHash(_board);
        for (int depth = 0; depth < maxDepth; ++depth)
        {
            // measuring time
            [[maybe_unused]]auto t1 = std::chrono::steady_clock::now();

            // preparing variables used to display statistics
            _currRootDepth = depth;
            _visitedNodes = 0;
            _cutoffNodes = 0;
            int eval{};

            // cleaning tables used in iteration
            _kTable.ClearPlyFloor(depth);

            // move list iteration
            for (size_t i = 0; i < moves.size; ++i)
            {
                Move::MakeMove(moves[i], _board);
                zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
                eval = -_negaScout(_board, NegativeInfinity, PositiveInfinity, depth, zHash, moves[i]);
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

                GlobalLogger.StartLogging() << std::format("info depth: {}, best move: {}, eval: {}, time: {}, nodes: {}, cut-off nodes: {},  nodes per sec: {}, tt entries: {}, at age: {}\n", depth + 1,
                                                           moves[0].GetLongAlgebraicNotation(), static_cast<double>(moves[0].GetEval())/100.0,
                                                           spentMs, _visitedNodes, _cutoffNodes,  nps, TTable.GetContainedElements(), _age);
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
    [[nodiscard]] int _alphaBeta(Board& bd, int alpha, int beta, int depthLeft, uint64_t zHash, Move prevMove);

    [[nodiscard]] int _negaScout(Board& bd, int alpha, int beta, int depthLeft, uint64_t zHash, Move prevMove);
    [[nodiscard]] int _quiescenceSearch(Board& bd, int alpha, int beta, uint64_t zHash);

    static void _embeddedMoveSort(MoveGenerator::payload moves, size_t range);
    static void _pullMoveToFront(MoveGenerator::payload moves, Move mv);
    static void _fetchBestMove(MoveGenerator::payload moves, size_t targetPos);

    [[nodiscard]] int _getMateValue(int depthLeft) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int MateMargin = 200;
    static constexpr int NegativeInfinity = INT16_MIN + 100;
    static constexpr int PositiveInfinity = INT16_MAX - 100;

    static constexpr uint16_t QuisenceAgeDiffToReplace = 12;
    static constexpr uint16_t SearchAgeDiffToReplace = 8;

    stack<Move, DefaultStackSize>& _stack;
    Board _board;
    const uint16_t _age;
    uint64_t _visitedNodes = 0;
    uint64_t _cutoffNodes = 0;
    int _currRootDepth = 0;
    KillerTable _kTable{};
    CounterMoveTable _cmTable{};
};

#endif  // BESTMOVESEARCH_H
