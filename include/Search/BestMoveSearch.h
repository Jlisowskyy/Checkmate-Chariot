//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include "../EngineTypeDefs.h"
#include "../Evaluation/CounterMoveTable.h"
#include "../Evaluation/HistoricTable.h"
#include "../Evaluation/KillerTable.h"
#include "../Interface/Logger.h"
#include "../ThreadManagement/stack.h"

class BestMoveSearch
{
    // ------------------------------
    // Class inner types
    // ------------------------------

    struct PV {
        PV() = delete;

        PV(const int depth): _depth(depth){}

        void InsertNext(const PackedMove mv, const PV& pv) {
            _path[0] = mv;
            memcpy(_path + 1, pv._path, (_depth-1)*sizeof(PackedMove));
        }

        void Clear() {
            memset(_path, 0, _depth*sizeof(PackedMove));
        }

        void Print() {
            for (int i = 0; i < _depth; ++i)
                GlobalLogger.StartLogging() << _path[i].GetLongAlgebraicNotation() << ' ';
        }

    private:
        PackedMove _path[MaxSearchDepth+1]{};
        const int _depth;
    };

public:
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

    void IterativeDeepening(PackedMove* output, int maxDepth, bool writeInfo = true);

    // ------------------------------
    // Private class methods
    // ------------------------------
   private:

    // ALPHA - minimum score of maximizing player
    // BETA - maximum score of minimizing player
    [[nodiscard]] int _pwsSearch(Board& bd, int alpha, int beta, int depthLeft, uint64_t zHash, Move prevMove, PV& pv);
    [[nodiscard]] int _zwSearch(Board& bd, int alpha, int depthLeft, uint64_t zHash, Move prevMove);
    [[nodiscard]] int _quiescenceSearch(Board& bd, int alpha, int beta, uint64_t zHash);
    [[nodiscard]] int _zwQuiescenceSearch(Board& bd, int alpha, uint64_t zHash);

    static void _embeddedMoveSort(stack<Move, DefaultStackSize>::stackPayload moves, size_t range);
    static void _pullMoveToFront(stack<Move, DefaultStackSize>::stackPayload moves, PackedMove mv);
    static void _fetchBestMove(stack<Move, DefaultStackSize>::stackPayload moves, size_t targetPos);

    [[nodiscard]] int _getMateValue(int depthLeft) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int MateMargin = 200;
    static constexpr int NegativeInfinity = INT16_MIN + 100;
    static constexpr int PositiveInfinity = INT16_MAX - 100;

    static constexpr uint16_t QuisenceAgeDiffToReplace = 16;
    static constexpr uint16_t SearchAgeDiffToReplace = 10;

    static constexpr int MaxAspWindowTries = 4;

    stack<Move, DefaultStackSize>& _stack;
    Board _board;
    const uint16_t _age;
    uint64_t _visitedNodes = 0;
    uint64_t _cutoffNodes = 0;
    int _currRootDepth = 0;
    KillerTable _kTable{};
    CounterMoveTable _cmTable{};
    HistoricTable _histTable{};
};

#endif  // BESTMOVESEARCH_H
