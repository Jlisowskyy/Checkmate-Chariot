//
// Created by Jlisowskyy on 7/13/24.
//

#include "../include/Search/MoveIterator.h"

#include <cassert>
#include <cinttypes>

Move MoveIterator::GetNextMove()
{
    Move rv{};

    do
    {
        // All moves were processed return immiediently
        if (_uncheckedMoves == 0)
            return {};

        // Act accordingally to current stage
        rv = _processStage();

        // Empty move signals that current stage was exhuasted without finding any move, retry again
    } while (rv.IsEmpty());

    --_uncheckedMoves;
    ++iters;

    if (iters > 256)
        GlobalLogger.LogStream << "XDDD";

    return rv;
}
Move MoveIterator::_processStage()
{
    switch (_stage)
    {
    case MoveSortStages::TT_MOVE:
        return _processTTMove();
    case MoveSortStages::GOOD_PROMOS:
        return _processGoodPromos();
    case MoveSortStages::GOOD_CAPTURES:
        return _processGoodCaptures();
    case MoveSortStages::KILLERS:
        return _processKillers();
    case MoveSortStages::GOOD_QUIETS:
        return _processGoodQuiets();
    case MoveSortStages::BAD_PROMOS:
        return _processBadPromos();
    case MoveSortStages::BAD_CAPTURES:
        return _processBadCaptures();
    case MoveSortStages::BAD_QUIETS:
        return _processBadQuiets();
    default:
        abort();
    }
}

Move MoveIterator::_processTTMove()
{
    Move retreivedTT{};

    // if this is first move to check, and we have some move from tt -> try to find it inisde the moves
    if (_uncheckedMoves == _moves.size && !_ttMove.IsEmpty())
        retreivedTT = _pullMove(
            [&](const Move mv)
            {
                return mv.GetPackedMove() == _ttMove;
            }
        );

    // if we found the move return immiediently
    if (!retreivedTT.IsEmpty())
        return retreivedTT;

    // if no move was found or we already returned it previously transition the state
    _stage = MoveSortStages::GOOD_PROMOS;
    _initPromos();

    // return empty move to signal state transition
    return {};
}
Move MoveIterator::_processGoodPromos()
{
    return _makeStep(
        [&]
        {
            _stage = MoveSortStages::GOOD_CAPTURES;
            _initCaptures();
        }, _currStageMoves);
}


Move MoveIterator::_processBadPromos()
{
    return _makeStep(
        [&]
        {
            _stage = MoveSortStages::BAD_CAPTURES;
        },
        _badPromos
    );
}
Move MoveIterator::_processGoodCaptures()
{
    return _makeStep(
        [&]
        {
            _stage = MoveSortStages::KILLERS;
        }, _currStageMoves);
}

Move MoveIterator::_processBadCaptures()
{
    return _makeStep(
        [&]
        {
            _stage = MoveSortStages::BAD_QUIETS;
        }, _badCaptures);
}

Move MoveIterator::_processKillers()
{
    if (_ply != 0)
    {
        const Move retreivedMove = _pullMove(
            [&](const Move mv)
            {
                return _kTable.IsKillerMove(mv, _ply);
            }
        );

        if (!retreivedMove.IsEmpty())
            return retreivedMove;
    }

    _stage = MoveSortStages::GOOD_QUIETS;
    _initQuiets();

    // Signal state change
    return {};
}

Move MoveIterator::_processGoodQuiets()
{
    return _makeStep(
        [&]
        {
            _stage = MoveSortStages::BAD_PROMOS;
        }, _currStageMoves
    );
}

Move MoveIterator::_processBadQuiets()
{
    return _makeStep([]{}, _badQuiets);
}

void MoveIterator::_initPromos()
{
    _initTables([](const Move mv)
        {
            return mv.GetPackedMove().IsPromo();
        },
        [&](Move mv)
        {
            // if the pawn is promoting and additionally capturing it is in most cases good move
            if (mv.IsAttackingMove())
            {
                mv.SetEval(_scoreCapture(mv) + mv.IsChecking() ? MOVE_SORT_PROMO_CHECK : 0);
                _currStageMoves.Push(mv);
            }
            else
            {
                const int seeValue = _generator.SEE(mv);

                // not worth promoting yet
                // TODO: what about enforcing figure moving (that is tactical aspect?)
                if (seeValue < 0)
                    _badPromos.Push(mv);
                else
                {
                    // enforce inspecting moves that give a check at first
                    mv.SetEval(mv.IsChecking() ? MOVE_SORT_PROMO_CHECK : 0);
                    _currStageMoves.Push(mv);
                }
            }
        }
    );
}
void MoveIterator::_initCaptures()
{
    _initTables(
        [](const Move mv)
        {
            return mv.GetPackedMove().IsCapture();
        },
        [&](Move mv)
        {
            // const auto score = static_cast<int16_t>(_scoreCapture(mv));
            // mv.SetEval(score);

            const int seeScore = _generator.SEE(mv);
            mv.SetEval(static_cast<int16_t>(seeScore));
            if (seeScore < -(MOVE_SORT_CAPTURE_COEF * mv.GetEval() / MOVE_SORT_CAPTURE_DIV  + MOVE_SORT_CAPTURE_BIAS))
                _badCaptures.Push(mv);
            else
                _currStageMoves.Push(mv);
        }
    );
}

void MoveIterator::_initQuiets()
{
    const uint64_t pawnAttacks =
        _board.MovingColor == WHITE
            ? BlackPawnMap::GetAttackFields(
                  _board.BitBoards[Board::BitBoardsPerCol * SwapColor(_board.MovingColor) + pawnsIndex]
              )
            : WhitePawnMap::GetAttackFields(
                  _board.BitBoards[Board::BitBoardsPerCol * SwapColor(_board.MovingColor) + pawnsIndex]
              );

    _initTables(
        [](const Move){ return true; },
        [&](Move mv)
        {
            const auto score = static_cast<int16_t>(_scoreQuiet(mv, pawnAttacks));
            mv.SetEval(score);

            if (score >= MOVE_SORT_GOOD_QUIET_SCORE)
                _currStageMoves.Push(mv);
            else
                _badQuiets.Push(mv);
        }
    );
}

template <size_t size>
Move MoveIterator::_moveStorage<size>::FetchBestMove()
{
    if (_counter == 0)
        return {};

    int maxValue  = NEGATIVE_INFINITY;
    size_t maxInd = 0;

    for (size_t i = 0; i < _counter; ++i)
    {
        if (const int heuristicEval = _table[i].GetEval(); heuristicEval > maxValue)
        {
            maxInd   = i;
            maxValue = heuristicEval;
        }
    }

    const Move rv = _table[maxInd];
    _table[maxInd] = _table[--_counter];
    return rv;
}

template <class PredT>
Move MoveIterator::_pullMove(PredT pred)
{
    for (size_t i = 0; i < _unscoredMoves; ++i)
        if (pred(_moves.data[i]))
        {
            const Move rv = _moves.data[i];
            _moves.data[i] = _moves.data[--_unscoredMoves];

            return rv;
        }

    return {};
}

int MoveIterator::_scoreCapture(const Move mv) const
{
    static constexpr int16_t FigureEval[] = {
        100, // wPawnsIndex,
        300, // wKnightsIndex,
        300, // wBishopsIndex,
        500, // wRooksIndex,
        900, // wQueensIndex,
        0,   // wKingIndex,
        100, // bPawnsIndex,
        300, // bKnightsIndex,
        300, // bBishopsIndex,
        500, // bRooksIndex,
        900, // bQueensIndex,
        0,   // bKingIndex,
    };

    return FigureEval[mv.GetKilledBoardIndex()] - FigureEval[mv.GetStartBoardIndex()];
}
int MoveIterator::_scoreQuiet(const Move mv, const uint64_t pawnAttacks) const
{
    int eval{};

    const PackedMove counterMove = _cTable.GetCounterMove(_prevMove, _board.MovingColor);

    // counter move table
    eval += COUNTER_MOVE_TABLE_PRIZE * (mv.GetPackedMove() == counterMove);

    // check bonus
    if (mv.IsChecking())
    {
        // validate if the check is worth at all
        const int seeValue = _generator.SEE(mv);

        if (seeValue > SEE_GOOD_MOVE_BOUNDARY)
            eval += MOVE_SORT_QUIET_CHECK;
    }

    // History table bonus
    eval += _hTable.GetBonusMove(mv);

    // pawn attacks evasion/danger
    eval += ((pawnAttacks & (MaxMsbPossible >> mv.GetStartField())) != 0) * MOVE_SORT_QUIETS_PAWN_EVASION_BONUS +
            ((pawnAttacks & (MaxMsbPossible >> mv.GetTargetField())) != 0) * MOVE_SORT_QUIETS_PAWN_DANGER_PENALTY;

    // continuation histories
    for (size_t i = 0; i < CONT_HISTORY_SCORE_TABLES_READ_COUNT; ++i)
        eval += _ctTables[i]->GetBonusMove(mv);

    return eval;
}