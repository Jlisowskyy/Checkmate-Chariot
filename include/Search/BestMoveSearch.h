//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include "../EngineUtils.h"
#include "../Evaluation/CounterMoveTable.h"
#include "../Evaluation/HistoricTable.h"
#include "../Evaluation/KillerTable.h"
#include "../Interface/Logger.h"
#include "../ThreadManagement/Stack.h"

class BestMoveSearch
{
    // ------------------------------
    // Class inner types
    // ------------------------------

    struct PV
    {
        PV() = delete;

        PV(const int depth) : _depth(depth) {}

        void InsertNext(const PackedMove mv, const PV &pv)
        {
            _path[0] = mv;
            memcpy(_path + 1, pv._path, (_depth - 1) * sizeof(PackedMove));
        }

        void Clear() { memset(_path, 0, _depth * sizeof(PackedMove)); }

        void Clone(const PV &pv) { memcpy(_path, pv._path, (_depth) * sizeof(PackedMove)); }

        void Print() const
        {
            for (int i = 0; i < _depth; ++i) GlobalLogger.LogStream << _path[i].GetLongAlgebraicNotation() << ' ';
        }

        PackedMove operator()(const int depthLeft, const int rootDepth) const { return _path[rootDepth - depthLeft]; }

        PackedMove operator[](const int ply) const { return _path[ply]; }

        private:
        PackedMove _path[MaxSearchDepth + 1]{};
        const int _depth;
    };

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    BestMoveSearch() = delete;
    BestMoveSearch(const Board &board, Stack<Move, DefaultStackSize> &s, const uint16_t age)
        : _stack(s), _board(board), _age(age)
    {
    }
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void IterativeDeepening(PackedMove *output, int maxDepth, bool writeInfo = true);

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    // ALPHA - minimum score of maximizing player
    // BETA - maximum score of minimizing player
    [[nodiscard]] int
    _pwsSearch(Board &bd, int alpha, int beta, int depthLeft, uint64_t zHash, Move prevMove, PV &pv, bool followPv);
    [[nodiscard]] int _zwSearch(Board &bd, int alpha, int depthLeft, uint64_t zHash, Move prevMove);
    [[nodiscard]] int _quiescenceSearch(Board &bd, int alpha, int beta, uint64_t zHash);
    [[nodiscard]] int _zwQuiescenceSearch(Board &bd, int alpha, uint64_t zHash);

    static void _embeddedMoveSort(Stack<Move, DefaultStackSize>::StackPayload moves, size_t range);
    static void _pullMoveToFront(Stack<Move, DefaultStackSize>::StackPayload moves, PackedMove mv);
    static void _fetchBestMove(Stack<Move, DefaultStackSize>::StackPayload moves, size_t targetPos);

    [[nodiscard]] int _getMateValue(int depthLeft) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr int ReservedValues                = 64;
    static constexpr int InfinityMargin                = MaxSearchDepth + ReservedValues;
    static constexpr int TimeStopValue                 = std::numeric_limits<int16_t>::max() - 10;
    static constexpr int NegativeInfinity              = std::numeric_limits<int16_t>::min() + InfinityMargin;
    static constexpr int PositiveInfinity              = std::numeric_limits<int16_t>::max() - InfinityMargin;
    static constexpr uint16_t QuisenceAgeDiffToReplace = 16;
    static constexpr uint16_t SearchAgeDiffToReplace   = 10;

    // Initial Aspiration Window Delta its cp value is equal to InitialAspWindowDelta * BoardEvaluator::ScoreGrain
    // (probably 8) ~= 48
    static constexpr int16_t InitialAspWindowDelta = 6;

    static constexpr int MaxAspWindowTries = 4;

    Stack<Move, DefaultStackSize> &_stack;
    Board _board;
    const uint16_t _age;
    uint64_t _visitedNodes = 0;
    uint64_t _cutoffNodes  = 0;
    int _currRootDepth     = 0;
    KillerTable _kTable{};
    CounterMoveTable _cmTable{};
    HistoricTable _histTable{};
};

#endif // BESTMOVESEARCH_H
