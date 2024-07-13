//
// Created by Jlisowskyy on 7/13/24.
//

#include "../include/Search/MoveIterator.h"

#include <cassert>

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
                return mv.GetPackedMove() == _ttMove.GetPackedMove();
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
        _badCaptures
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
    const Move retreivedMove = _pullMove(
        [&](const Move mv)
        {
            return _kTable.IsKillerMove(mv, _ply);
        }
    );

    if (!retreivedMove.IsEmpty())
        return retreivedMove;

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
                mv.SetEval(_scoreCapture(mv) + mv.IsChecking() ? 100 : 0);
                _currStageMoves.Push(mv);
            }
            else
            {
                const int seeValue = _generator.SEE(mv);

                // not worth promoting yet
                // TODO: what about enforcing figure moving (that is tactical aspect?)
                if (seeValue < 0)
                    _badCaptures.Push(mv);
                else
                {
                    // enforce inspecting moves that give a check at first
                    mv.SetEval(mv.IsChecking() ? 100 : 0);
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

        }
    );
}

void MoveIterator::_initQuiets()
{
    _initTables(
        [](const Move mv){ return true; },
        [&](Move mv)
        {

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

int MoveIterator::_scoreCapture(const Move mv)
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