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

/*
 * Class defines our search algorithm.
 * It uses PVS (Principal Variation Search) with aspiration windows combined with iterative deepening as a main
 * framework. All is built in so call NegaMax framework.
 *
 * References:
 * - PVS: https://www.chessprogramming.org/Principal_Variation_Search
 * - PV PATH: https://www.chessprogramming.org/Principal_Variation
 * - zero window search: https://www.chessprogramming.org/Null_Window
 * - alpha-beta: https://www.chessprogramming.org/Alpha-Beta
 * - aspiration windows: https://www.chessprogramming.org/Aspiration_Windows
 * - iterative deepening: https://www.chessprogramming.org/Iterative_Deepening
 * - Transposition table: https://www.chessprogramming.org/Transposition_Table
 * - move ordering: https://www.chessprogramming.org/Move_Ordering
 * - quiescence search: https://www.chessprogramming.org/Quiescence_Search
 * - NegaMax: https://www.chessprogramming.org/Negamax, https://www.wikipedia.org/wiki/Negamax
 *
 *
 * Other used features:
 * - Transposition table
 * - move ordering
 * - quiescence search
 * - pv saving and following
 *
 * Given 'depth' parameter defines how many layers should be searched more. When referring to depth in the code, that
 * means that the search tree grows from bottom to top.
 *
 * When referring to ply search tree, it grows from top to bottom.
 * */

class BestMoveSearch
{
    // ------------------------------
    // Class inner types
    // ------------------------------

    /*
     * Class defines container for Principal Variation path. That means the path of moves that are considered to be the
     * best ones in the current position.
     *
     * It defines all necessary methods to work with the path.
     *
     * It stores the path in the array of PackedMoves and the depth of the path.
     * All operations depend on the depth of the path stored internally.
     *
     * */

    struct PV
    {
        PV() = default;
        explicit PV(const int depth) : _depth(depth) {}

        /*
         * Inserts the given move 'mv' as af first in the array and then pastes the rest of the other PV path 'pv',
         * according to internal depth field.
         * */

        void InsertNext(const PackedMove mv, const PV &pv)
        {
            _path[0] = mv;
            memcpy(_path + 1, pv._path, (_depth - 1) * sizeof(PackedMove));
        }

        void SetDepth(const int depth) { _depth = depth; }

        /* Set a new depth and clears the path on that depth range */
        void Clear(const int nDepth)
        {
            SetDepth(nDepth);
            memset(_path, 0, _depth * sizeof(PackedMove));
        }

        /* Clones the path from the given PV*/
        void Clone(const PV &pv)
        {
            _depth = pv._depth;
            memcpy(_path, pv._path, (_depth) * sizeof(PackedMove));
        }

        /* Prints the path to the Logger */
        void Print() const
        {
            std::string buff{};
            for (int i = 0; i < _depth; ++i) buff += _path[i].GetLongAlgebraicNotation() + ' ';

            GlobalLogger.LogStream << buff;
        }

        /* Debug function to check internal state of the PV */
        [[nodiscard]] bool IsFilled() const
        {
            for (int i = 0; i < _depth; ++i)
                if (_path[i].IsEmpty())
                    return false;

            return true;
        }

        /* Returns the move on the given depth, with respect to given 'rootDepth' where depth is same as in search
         * function */
        PackedMove operator()(const int depthLeft, const int rootDepth) const { return _path[rootDepth - depthLeft]; }

        /* returns the move */
        PackedMove operator[](const int ply) const { return _path[ply]; }

        private:
        PackedMove _path[MaxSearchDepth + 1]{};
        int _depth{1};
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

    void IterativeDeepening(PackedMove *bestMove, PackedMove *ponderMove, int maxDepth, bool writeInfo = true);

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
    PV _pv{};
    const uint16_t _age;
    uint64_t _visitedNodes = 0;
    uint64_t _cutoffNodes  = 0;
    int _currRootDepth     = 0;
    KillerTable _kTable{};
    CounterMoveTable _cmTable{};
    HistoricTable _histTable{};
};

#endif // BESTMOVESEARCH_H
