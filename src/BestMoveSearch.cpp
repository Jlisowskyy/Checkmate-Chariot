//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/Search/BestMoveSearch.h"

#include "../include/Evaluation/BoardEvaluator.h"


int BestMoveSearch::_alphaBeta(Board& bd, int alpha, int beta, const int depthLeft, uint64_t zHash, const Move prevMove)
{
    // prefetching table record
    TTable.Prefetch(zHash);

    const int alphaStart = alpha;
    Move bestMove;
    int bestEval = NegativeInfinity;
    bool wasTTHit = false;
    nodeType nType = pvNode;

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _quiescenceSearch(bd, alpha, beta, zHash);

    // incrementing nodes counter;
    ++_visitedNodes;

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    // reading Transposition table for previous score
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    if (prevSearchRes.GetHash() == zHash)
    {
        wasTTHit = true;

        if (prevSearchRes.GetDepth() >= depthLeft)
        {
            switch (prevSearchRes.GetNodeType())
            {
                case pvNode:
                    return prevSearchRes.GetEval();
                case lowerBound:
                    alpha = std::max(alpha, prevSearchRes.GetEval());
                    break;
                case upperBound:
                    beta = std::min(beta, prevSearchRes.GetEval());
                    break;
                default:
                    break;
            }

            if (alpha > beta)
            {
                ++_cutoffNodes;
                return prevSearchRes.GetEval();
            }
        }
    }

    // generate moves
    MoveGenerator mechanics(bd, _stack, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField());
    auto moves = mechanics.GetMovesFast();

    // signalize checks and draws
    if (moves.size == 0)
    {
        _stack.PopAggregate(moves);
        return mechanics.IsCheck() ? _getMateValue(depthLeft) : 0;
    }

    // processsing each move
    for (size_t i = 0; i < moves.size; ++i)
    {
        // load best move from TT if possible
        if (i == 0 && wasTTHit)
            _pullMoveToFront(moves, prevSearchRes.GetMove());
        else
            _fetchBestMove(moves, i);

        // processing move
        Move::MakeMove(moves[i], bd);
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        _kTable.ClearPlyFloor(depthLeft - 1);
        const int moveEval = -_alphaBeta(bd, -beta, -alpha, depthLeft - 1, zHash, moves[i]);
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

        if (moveEval > bestEval)
        {
            bestEval = moveEval;
            bestMove = moves[i];

            // cut-off found
            if (moveEval >= beta)
            {
                if (moves[i].IsAttackingMove() == false)
                {
                    if (moves[i].IsQuietMove())
                        _kTable.SaveKillerMove(moves[i], depthLeft);

                    _cmTable.SaveCounterMove(moves[i].GetPackedMove(), prevMove);
                }

                nType = lowerBound;

                ++_cutoffNodes;
                break;
            }

            // updating alpha
            alpha = std::max(alpha, moveEval);
        }


    }

    //clean up
    _stack.PopAggregate(moves);

    // updating if profitable
    if (depthLeft >= prevSearchRes.GetDepth() || (wasTTHit == false && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        const int statVal = wasTTHit ? prevSearchRes.GetStatVal() : TranspositionTable::HashRecord::NoEval;
        nType = bestEval <= alphaStart ? upperBound : nType;

        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statVal, depthLeft, nType, _age };
        TTable.Add(record);
    }

    return bestEval;
}

int BestMoveSearch::_negaScout(Board& bd, int alpha, int beta, const int depthLeft, uint64_t zHash, const Move prevMove)
{
    TTable.Prefetch(zHash);

    const int alphaStart = alpha;
    Move bestMove;
    int bestEval = NegativeInfinity;
    bool wasTTHit = false;
    nodeType nType = pvNode;

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _quiescenceSearch(bd, alpha, beta, zHash);
    // incrementing nodes counter;
    ++_visitedNodes;

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    // reading Transposition table for previous score
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    if (prevSearchRes.GetHash() == zHash)
    {
        wasTTHit = true;
        if (prevSearchRes.GetDepth() >= depthLeft)
        {
            switch (prevSearchRes.GetNodeType())
            {
                case pvNode:
                    return prevSearchRes.GetEval();
                case lowerBound:
                    alpha = std::max(alpha, prevSearchRes.GetEval());
                    break;
                case upperBound:
                    beta = std::min(beta, prevSearchRes.GetEval());
                    break;
                default:
                    break;
            }

            if (alpha > beta)
            {
                ++_cutoffNodes;
                return prevSearchRes.GetEval();
            }
        }
    }

    // generate moves
    MoveGenerator mechanics(bd, _stack, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField());
    auto moves = mechanics.GetMovesFast();

    // processsing each move
    for (size_t i = 0; i < moves.size; ++i)
    {
        // load best move from TT if possible
        if (i == 0 && wasTTHit)
            _pullMoveToFront(moves, prevSearchRes.GetMove());
        else
            _fetchBestMove(moves, i);

        int moveEval;
        Move::MakeMove(moves[i], bd);
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        if (i == 0)
        {
            // processing assumed pvs move
            _kTable.ClearPlyFloor(depthLeft - 1);
            moveEval = -_negaScout(bd, -beta, -alpha, depthLeft - 1, zHash, moves[i]);
        }
        else
        {
            // performing checks wheter assumed thesis holds
            _kTable.ClearPlyFloor(depthLeft - 1);
            moveEval = -_negaScout(bd, -alpha - 1, -alpha, depthLeft - 1, zHash, moves[i]);

            // if not research move
            if (alpha < moveEval && moveEval < beta)
            {
                _kTable.ClearPlyFloor(depthLeft - 1);
                moveEval = -_negaScout(bd, -beta, -alpha, depthLeft - 1, zHash, moves[i]);
            }
        }
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

        if (moveEval > bestEval)
        {
            bestEval = moveEval;
            bestMove = moves[i];

            // cut-off found
            if (moveEval >= beta)
            {
                if (moves[i].IsAttackingMove() == false)
                {
                    if (moves[i].IsQuietMove())
                        _kTable.SaveKillerMove(moves[i], depthLeft);

                    _cmTable.SaveCounterMove(moves[i].GetPackedMove(), prevMove);
                }
                nType = lowerBound;

                ++_cutoffNodes;
                break;
            }

            // updating alpha
            alpha = std::max(alpha, moveEval);
        }
    }

    //clean up
    _stack.PopAggregate(moves);

    // updating if profitable
    if (depthLeft >= prevSearchRes.GetDepth() || (wasTTHit == false && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        nType = bestEval <= alphaStart ? upperBound : nType;

        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, prevSearchRes.GetStatVal(), depthLeft, nType, _age };
        TTable.Add(record);
    }

    return bestEval;
}

int BestMoveSearch::_quiescenceSearch(Board& bd, int alpha, int beta, uint64_t zHash)
{
    int bestEval = NegativeInfinity;
    ++_visitedNodes;

    const int eval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.movColor);
    if (eval >= beta)
    {
        ++_cutoffNodes;
        return beta;
    }
    alpha = std::max(alpha, eval);

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

    // end of path
    if (moves.size == 0)
        return eval;

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        _fetchBestMove(moves, i);

        Move::MakeMove(moves[i], bd);
        const int moveValue = -_quiescenceSearch( bd, -beta, -alpha, zHash);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

        if (moveValue > bestEval)
        {
            bestEval = moveValue;

            if (moveValue >= beta)
            {
                ++ _cutoffNodes;
                break;
            }

            alpha = std::max(alpha, moveValue);
        }
    }

    // clean up
    _stack.PopAggregate(moves);
    return bestEval;
}

void BestMoveSearch::_embeddedMoveSort(MoveGenerator::payload moves, const size_t range)
{
    for (ssize_t i = 1; i < static_cast<ssize_t>(range); ++i)
    {
        ssize_t j = i - 1;
        const auto val = moves[i];
        while (j >= 0 && moves[j].GetEval() < val.GetEval())
        {
            moves.data[j + 1] = moves[j];
            j--;
        }
        moves.data[j + 1] = val;
    }
}

void BestMoveSearch::_pullMoveToFront(MoveGenerator::payload moves, const Move mv)
{
    // preparing sentinel
    const Move sentinelOld = moves.data[moves.size];
    moves.data[moves.size] = mv;

    // finding stopping index
    size_t ind = 0;
    while(moves.data[ind] != mv) ind++;

    // replacing old element
    moves.data[moves.size] = sentinelOld;

    // if move found swapping
    if (ind != moves.size)
    {
        // // moving moves to preserve sorted order
        // while (ind != 0)
        // {
        //     moves.data[ind] = moves.data[ind - 1];
        //     ind--;
        // }
        // moves.data[0] = mv;
        std::swap(moves.data[ind], moves.data[0]);
    }
}

void BestMoveSearch::_fetchBestMove(MoveGenerator::payload moves, const size_t targetPos)
{
    int maxValue = NegativeInfinity;
    size_t maxInd = targetPos;

    for (size_t i = targetPos; i < moves.size; ++i)
    {
        if (const int heuresticEval = moves[i].GetEval(); heuresticEval > maxValue)
        {
            maxInd = i;
            maxValue = heuresticEval;
        }
    }

    std::swap(moves.data[maxInd], moves.data[targetPos]);
}

int BestMoveSearch::_getMateValue(const int depthLeft) const
{
    const int distToRoot = _currRootDepth - depthLeft;
    return NegativeInfinity + distToRoot;
}
