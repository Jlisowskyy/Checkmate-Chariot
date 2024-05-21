//
// Created by Jlisowskyy on 2/29/24.
//

#ifndef BESTMOVESEARCH_H
#define BESTMOVESEARCH_H

#include <map>

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
    using RepMap = std::map<uint64_t, int>;

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
        PackedMove _path[MAX_SEARCH_DEPTH + 1]{};
        int _depth{1};
    };

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    /*
     * Construction needs a board as starting state of the search algorithm,
     * Stack as a container to store moves and age to use inside the TT replacement scheme.
     *
     * */

    BestMoveSearch() = delete;
    BestMoveSearch(const Board &board, const RepMap& rMap, Stack<Move, DEFAULT_STACK_SIZE> &s, const uint16_t age)
        : _stack(s), _board(board), _repMap(rMap), _age(age)
    {
    }
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    /*
     * Main search functions. Perform searches going through every depth one by one
     * to collect information about shallower nodes. It is profitable, because average chess position tree expansion
     * rate is +/- 40. Let 'd' be the current depth and r = '40' to be tree growth rate. So cost to go through every
     * previous depth is: cost(d) = 1/(40^(d-1)) + 1/(40^(d-2)) + ... +  1/(40^(d)) ~= 1/r that is in most cases
     * such operation costs us only 1/40 ~= 2% of whole time processing. In short, it is quite profitable overall.
     *
     * Input:
     *  - maxDepth - limits the depth of the search,
     *  - writeInfo - defines whether info should be displayed
     *  Return:
     *  - bestMove - best move found ready to play,
     *  - ponderMove - move that is considered to be likely play as a response to our move
     *
     * */

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

    static void _embeddedMoveSort(Stack<Move, DEFAULT_STACK_SIZE>::StackPayload moves, size_t range);
    static void _pullMoveToFront(Stack<Move, DEFAULT_STACK_SIZE>::StackPayload moves, PackedMove mv);
    static void _fetchBestMove(Stack<Move, DEFAULT_STACK_SIZE>::StackPayload moves, size_t targetPos);

    [[nodiscard]] int _getMateValue(int depthLeft) const;
    [[nodiscard]] INLINE bool _isDrawByReps(const uint64_t hash){
        return _repMap[hash] >= 3 || _board.HalfMoves >= 50;
    }

    void INLINE _saveQuietMoveInfo(const Move mv, const Move prevMove, const int depth)
    {
        _kTable.SaveKillerMove(mv, depth);
        _cmTable.SaveCounterMove(mv.GetPackedMove(), prevMove);
        _histTable.SetBonusMove(mv, depth);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    Stack<Move, DEFAULT_STACK_SIZE> &_stack;
    Board _board;
    RepMap _repMap;
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
