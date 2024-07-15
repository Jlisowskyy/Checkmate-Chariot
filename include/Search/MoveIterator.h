//
// Created by Jlisowskyy on 7/13/24.
//

#ifndef MOVEITERATOR_H
#define MOVEITERATOR_H
#include "../Board.h"
#include "../Evaluation/CounterMoveTable.h"
#include "../Evaluation/HistoricTable.h"
#include "../Evaluation/KillerTable.h"
#include "../MoveGeneration/MoveGenerator.h"


/* Class used to sort moves on the fly, accordingaly to strictly defined rules */
class MoveIterator
{
    enum class MoveSortStages
    {
        TT_MOVE,
        GOOD_PROMOS,
        GOOD_CAPTURES,
        KILLERS,
        GOOD_QUIETS,
        BAD_PROMOS,
        BAD_CAPTURES,
        BAD_QUIETS,
    };

    template<size_t size>
    struct _moveStorage
    {
        void Push(Move mv) { _table[_counter++] = mv; }
        [[nodiscard]] size_t Range() const { return _counter; }
        [[nodiscard]] Move operator[](size_t ind) const { return _table[ind]; }
        void Clear() { _counter = 0; }

        [[nodiscard]] Move FetchBestMove();

        private:
        Move _table[size]{};
        size_t _counter{};
    };

    // ------------------------------
    // Class creation
    // ------------------------------
    public:

    MoveIterator(const Board &board, MoveGenerator &generator, MoveGenerator::payload moves, int plyDepth, int ply, Move prevMove, PackedMove ttMove,
        KillerTable& kTable, HistoricTable& hTable, CounterMoveTable& cTable, const HistoricTable** ctTables)
    : _board(board), _generator(generator), _moves(moves), _depth(plyDepth), _ply(ply), _prevMove(prevMove), _ttMove(ttMove), _kTable(kTable),
        _hTable(hTable), _cTable(cTable), _ctTables(ctTables)
    {
        _uncheckedMoves = _unscoredMoves = moves.size;
    }

    ~MoveIterator() = default;

    MoveIterator() = delete;
    MoveIterator(const MoveIterator&) = delete;
    MoveIterator(MoveIterator&&) = delete;

    MoveIterator& operator=(const MoveIterator&) = delete;
    MoveIterator& operator=(MoveIterator&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    Move GetNextMove();

    // ------------------------------
    // Private class methods
    // ------------------------------
    private:

    Move _processStage();

    Move _processTTMove();
    Move _processGoodPromos();
    Move _processBadPromos();
    Move _processGoodCaptures();
    Move _processBadCaptures();
    Move _processKillers();
    Move _processGoodQuiets();
    Move _processBadQuiets();

    void _initPromos();
    void _initCaptures();
    void _initQuiets();

    template <class PredT>
    [[nodiscard]] Move _pullMove(PredT pred);
    [[nodiscard]] int _scoreCapture(Move mv) const;
    [[nodiscard]] int _scoreQuiet(Move mv, uint64_t pawnAttacks) const;

    template <class PostActionT, class StorageT>
    INLINE static Move _makeStep(PostActionT action, StorageT& storage)
    {
        if (const Move fetchedMove = storage.FetchBestMove(); !fetchedMove.IsEmpty())
            return fetchedMove;

        // if no move was retreived, run the post action
        action();

        // signal state transition
        return {};
    }

    template<class ExtractionPredT, class ActionT>
    INLINE void _initTables(ExtractionPredT extraction, ActionT action)
    {
        Move _retrievedFigure = _pullMove(extraction);

        // Extract all figures
        while (!_retrievedFigure.IsEmpty())
        {
            action(_retrievedFigure);
            _retrievedFigure = _pullMove(extraction);
        }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    // Amount of maximally available pawns
    static constexpr size_t MAX_BAD_PROMOS = 8;

    // 10 qeeuns, ...
    static constexpr size_t MAX_BAD_CAPTURES = 112;
    static constexpr size_t MAX_BAD_QUEITS = 256;

    // Comes from maximum number of captures
    static constexpr size_t MAX_CURR_TACTICAL = 112;

    const Board& _board;
    MoveGenerator& _generator;
    MoveGenerator::payload _moves;
    const int _depth;
    const int _ply;
    const Move _prevMove;
    const PackedMove _ttMove;

    const KillerTable& _kTable;
    const HistoricTable& _hTable;
    const CounterMoveTable& _cTable;
    const HistoricTable** _ctTables;

    MoveSortStages _stage { MoveSortStages::TT_MOVE };
    size_t _uncheckedMoves{};
    size_t _unscoredMoves{};

    // Stores move to use in later stage
    _moveStorage<MAX_BAD_PROMOS> _badPromos{};
    _moveStorage<MAX_BAD_CAPTURES> _badCaptures{};
    _moveStorage<MAX_BAD_QUEITS> _badQuiets{};
    _moveStorage<MAX_CURR_TACTICAL> _currStageMoves{};

    size_t iters{};
};

#endif //MOVEITERATOR_H
