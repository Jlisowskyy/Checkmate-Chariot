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
#include "../Evaluation/BoardEvaluator.h"
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
        const uint64_t zHash = ZHasher.GenerateHash(_board);
        int eval{};

        for (int depth = 1; depth < maxDepth; ++depth)
        {
            // measuring time
            [[maybe_unused]]auto t1 = std::chrono::steady_clock::now();

            // preparing variables used to display statistics
            _currRootDepth = depth;
            _visitedNodes = 0;
            _cutoffNodes = 0;

            // cleaning tables used in iteration

            if (depth < 4)
            {
                _kTable.ClearPlyFloor(depth);
                eval = _negaScout(_board, NegativeInfinity, PositiveInfinity, depth, zHash, {});
            }
            else
            {
                int delta = BoardEvaluator::BasicFigureValues[wPawnsIndex] / 4;
                int alpha = eval - delta;
                int beta = eval + delta;

                while (true)
                {
                    _kTable.ClearPlyFloor(depth);
                    eval = _negaScout(_board, alpha, beta, depth, zHash, {});

                    if (eval <= alpha)
                    {
                        beta = (alpha + beta) / 2;
                        alpha = std::max(eval - delta, NegativeInfinity);
                    }
                    else if (eval >= beta)
                        beta = std::min(eval + delta, PositiveInfinity);
                    else
                        break;

                    delta += delta;
                }

            }

            // measurment end
            [[maybe_unused]]auto t2 = std::chrono::steady_clock::now();

            // move sorting
            auto record = TTable.GetRecord(zHash);
            *output = record.GetMove();

            if constexpr (WriteInfo)
            {
                static constexpr uint64_t MSEC = 1000 * 1000; // in nsecs
                const uint64_t spentMs = std::max(1LU, (t2-t1).count()/MSEC);
                const uint64_t nps = 1000LLU * _visitedNodes / spentMs;
                const double cutOffPerc = static_cast<double>(_cutoffNodes)/static_cast<double>(_visitedNodes);

                GlobalLogger.StartLogging() << std::format("info depth: {}, best move: {}, eval: {}, time: {}, nodes: {}, cut-off nodes: {}, with succeess rate: {}, nodes per sec: {}, tt entries: {}, at age: {}\n", depth + 1,
                                                           output->GetLongAlgebraicNotation(), static_cast<double>(eval)/100.0,
                                                           spentMs, _visitedNodes, _cutoffNodes, cutOffPerc,  nps, TTable.GetContainedElements(), _age);
            }
        }
    }

    // ------------------------------
    // Private class methods
    // ------------------------------
   private:
    // ALPHA - minimum score of maximizing player
    // BETA - maximum score of minimazing player
    [[nodiscard]] int _negaScout(Board& bd, int alpha, int beta, int depthLeft, uint64_t zHash, Move prevMove);
    [[nodiscard]] int _zwSearch(Board& bd, int alpha, int depthLeft, uint64_t zHash, Move prevMove);
    [[nodiscard]] int _quiescenceSearch(Board& bd, int alpha, int beta, uint64_t zHash);
    [[nodiscard]] int _zwQuiescenceSearch(Board& bd, int alpha, uint64_t zHash);

    static void _embeddedMoveSort(MoveGenerator::payload moves, size_t range);
    static void _pullMoveToFront(MoveGenerator::payload moves, PackedMove mv);
    static void _fetchBestMove(MoveGenerator::payload moves, size_t targetPos);

    [[nodiscard]] int _getMateValue(int depthLeft) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int MateMargin = 200;
    static constexpr int NegativeInfinity = INT16_MIN + 100;
    static constexpr int PositiveInfinity = INT16_MAX - 100;

    static constexpr uint16_t QuisenceAgeDiffToReplace = 16;
    static constexpr uint16_t SearchAgeDiffToReplace = 10;

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
