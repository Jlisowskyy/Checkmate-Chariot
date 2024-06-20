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

        /*
         * Inserts the given move 'mv' as af first in the array and then pastes the rest of the other PV path 'pv',
         * according to internal depth field.
         * */

        INLINE void InsertNext(const PackedMove mv, const PV &pv)
        {
            _path[0] = mv;
            memcpy(_path + 1, pv._path, pv._depth * sizeof(PackedMove));
            _depth = 1 + pv._depth;
        }

        /* Clones the path from the given PV*/
        INLINE void Clone(const PV &pv)
        {
            _depth = pv._depth;
            memcpy(_path, pv._path, pv._depth * sizeof(PackedMove));
        }

        /* Prints the path to the Logger */
        INLINE void Print(const bool isDraw) const
        {
            std::string buff{};

            if (!isDraw)
                // when no draw was detected we expect null moves to be printed to simplify debugging
                for (int i = 0; i < _depth; ++i) buff += _path[i].GetLongAlgebraicNotation() + ' ';
            else
                // We do not allow null moves when there was some draw detected
                for (int i = 0; i < _depth && !_path[i].IsEmpty(); ++i)
                    buff += _path[i].GetLongAlgebraicNotation() + ' ';

            GlobalLogger.LogStream << buff;
        }

        [[nodiscard]] INLINE bool Contains(int ply) const
        {
            return ply < _depth;
        }

        /* Debug function to check internal state of the PV */
        [[nodiscard]] INLINE bool IsFilled() const
        {
            for (int i = 0; i < _depth; ++i)
                if (_path[i].IsEmpty())
                    return false;

            return true;
        }

        /* returns the move */
        INLINE PackedMove operator[](const int ply) const { return _path[ply]; }

        private:
        PackedMove _path[MAX_SEARCH_DEPTH + 1]{};
        int _depth{};
    };

    enum class SearchType
    {
        PVSearch,
        NoPVSearch
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
    BestMoveSearch(const Board &board, Stack<Move, DEFAULT_STACK_SIZE> &s) : _stack(s), _board(board) {}
    ~BestMoveSearch() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    /*
     * Main search functions. Perform searches going through every depth one by one
     * to collect information about shallower nodes. It is profitable, because average chess position tree expansion
     * rate is +/- 40. Let 'd' be the current depth and r = '40' to be tree growth rate. So cost to go through every
     * previous depth is: cost(d) = 1/(40^(1)) + 1/(40^(2)) + ... +  1/(40^(d-1)) ~= 1/r that is in most cases
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

    int IterativeDeepening(PackedMove *bestMove, PackedMove *ponderMove, int maxDepth, bool writeInfo = true);
    int QuiesceEval();

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    // ALPHA - minimum score of maximizing player
    // BETA - maximum score of minimizing player

    template <SearchType searchType, bool followPv>
    int _search(int alpha, int beta, int depthLeft, int ply, uint64_t zHash, Move prevMove, PV &pv, PackedMove* bestMoveOut);

    template <SearchType searchType> int _qSearch(int alpha, int beta, int ply, uint64_t zHash, int extendedDepth);

    static void _pullMoveToFront(Stack<Move, DEFAULT_STACK_SIZE>::StackPayload moves, PackedMove mv);
    static void _fetchBestMove(Stack<Move, DEFAULT_STACK_SIZE>::StackPayload moves, size_t targetPos);

    void INLINE _saveQuietMoveInfo(const Move mv, const Move prevMove, const int depth, const int ply)
    {
        _kTable.SaveKillerMove(mv, ply);
        _cmTable.SaveCounterMove(mv.GetPackedMove(), prevMove);
        _histTable.SetBonusMove(mv, depth);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    Stack<Move, DEFAULT_STACK_SIZE> &_stack;
    Board _board;
    PV _pv{};
    PV _dummyPv{};
    uint64_t _visitedNodes = 0;
    uint64_t _cutoffNodes  = 0;
    KillerTable _kTable{};
    CounterMoveTable _cmTable{};
    HistoricTable _histTable{};
};

#endif // BESTMOVESEARCH_H
