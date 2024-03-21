//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/Search/BestMoveSearch.h"

#include <cassert>

#include "../include/Evaluation/BoardEvaluator.h"

int BestMoveSearch::_negaScout(Board& bd, int alpha, int beta, const int depthLeft, uint64_t zHash, const Move prevMove)
{
    assert(alpha < beta);

    nodeType nType = upperBound;
    Move bestMove{};

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _quiescenceSearch(bd, alpha, beta, zHash);

    // incrementing nodes counter;
    ++_visitedNodes;

    // generate moves
    MoveGenerator mechanics(bd, _stack, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField());
    auto moves = mechanics.GetMovesFast();

    if (moves.size == 0)
        return mechanics.IsCheck() ? _getMateValue(depthLeft) : 0;

    // reading Transposition table for best move
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    if (prevSearchRes.GetHash() == zHash && prevSearchRes.GetNodeType() != upperBound)
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    zHash = ZHasher.UpdateHash(zHash, moves[0], oldElPassant, oldCastlings);
    TTable.Prefetch(zHash);
    Move::MakeMove(moves[0], bd);
    _kTable.ClearPlyFloor(depthLeft - 1);
    int bestEval = -_negaScout(bd, -beta, -alpha, depthLeft - 1, zHash, moves[0]);
    zHash = ZHasher.UpdateHash(zHash, moves[0], oldElPassant, oldCastlings);
    Move::UnmakeMove(moves[0], bd, oldCastlings, oldElPassant);

    if (bestEval >= alpha)
    {
        nType = pvNode;

        // cut-off found
        if (bestEval >= beta)
        {
            ++_cutoffNodes;

            if (moves[0].IsAttackingMove() == false)
            {
                if (moves[0].IsQuietMove())
                    _kTable.SaveKillerMove(moves[0], depthLeft);

                _cmTable.SaveCounterMove(moves[0].GetPackedMove(), prevMove);
            }

            // updating if profitable
            if (depthLeft >= prevSearchRes.GetDepth()
                || (prevSearchRes.GetHash() != zHash && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
            {
                const TranspositionTable::HashRecord record{zHash, moves[0], bestEval, prevSearchRes.GetStatVal(), depthLeft, lowerBound, _age };
                TTable.Add(record);
            }

            _stack.PopAggregate(moves);
            return bestEval;
        }

        alpha = bestEval;
        bestMove = moves[0];
    }

    // processsing each move
    for (size_t i = 1; i < moves.size; ++i)
    {
        _fetchBestMove(moves, i);

        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        TTable.Prefetch(zHash);
        Move::MakeMove(moves[i], bd);

        // performing checks wheter assumed thesis holds
        _kTable.ClearPlyFloor(depthLeft - 1);
        int moveEval = -_zwSearch(bd, -alpha -1, depthLeft - 1, zHash, moves[i]);

        // if not research move
        if (alpha < moveEval && moveEval < beta)
        {
            TTable.Prefetch(zHash);
            _kTable.ClearPlyFloor(depthLeft - 1);
            moveEval = -_negaScout(bd, -beta, -alpha, depthLeft - 1, zHash, moves[i]);

            if (moveEval > bestEval)
            {
                bestEval = moveEval;

                if (moveEval >= alpha)
                {
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
                        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
                        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);
                        break;
                    }

                    nType = pvNode;
                }
            }
        }
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);
    }

    //clean up
    _stack.PopAggregate(moves);

    // updating if profitable
    if (depthLeft >= prevSearchRes.GetDepth()
        || (prevSearchRes.GetHash() != zHash && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, prevSearchRes.GetStatVal(), depthLeft, nType, _age };
        TTable.Add(record);
    }

    return bestEval;
}

[[nodiscard]] int BestMoveSearch::_zwSearch(Board& bd, const int alpha, const int depthLeft, uint64_t zHash, const Move prevMove) {
    const int beta = alpha + 1;
    int bestEval = NegativeInfinity;
    nodeType nType = upperBound;
    Move bestMove {};

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _zwQuiescenceSearch(bd, alpha, zHash);

    // incrementing nodes counter;
    ++_visitedNodes;

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    // reading Transposition table for previous score
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    if (prevSearchRes.GetHash() == zHash && prevSearchRes.GetDepth() >= depthLeft)
    {
        const nodeType expectedType = prevSearchRes.GetEval() >= beta ? lowerBound : upperBound;

        if (expectedType == prevSearchRes.GetNodeType() || prevSearchRes.GetNodeType() == pvNode)
        {
            ++_cutoffNodes;
            return prevSearchRes.GetEval();
        }
    }

    // generate moves
    MoveGenerator mechanics(bd, _stack, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField());
    auto moves = mechanics.GetMovesFast();

    if (moves.size == 0)
        return mechanics.IsCheck() ? _getMateValue(depthLeft) : 0;

    // processsing each move
    for (size_t i = 0; i < moves.size; ++i)
    {
        // load best move from TT if possible
        if (i == 0 && prevSearchRes.GetHash() == zHash && prevSearchRes.GetNodeType() != upperBound)
            _pullMoveToFront(moves, prevSearchRes.GetMove());
        else
            _fetchBestMove(moves, i);

        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        TTable.Prefetch(zHash);
        Move::MakeMove(moves[i], bd);
        _kTable.ClearPlyFloor(depthLeft - 1);
        const int moveEval = -_zwSearch(bd, -beta, depthLeft - 1, zHash, moves[i]);
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

        if (moveEval > bestEval)
        {
            bestEval = moveEval;

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
                bestMove = moves[i];
                ++_cutoffNodes;
                break;
            }
        }
    }

    if (depthLeft >= prevSearchRes.GetDepth()
            || (prevSearchRes.GetHash() == zHash && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, prevSearchRes.GetStatVal(), depthLeft, nType, _age };
        TTable.Add(record);
    }

    //clean up
    _stack.PopAggregate(moves);
    return bestEval;
}

int BestMoveSearch::_quiescenceSearch(Board& bd, int alpha, int beta, uint64_t zHash)
{
    assert(beta > alpha);

    ++_visitedNodes;
    int bestEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.movColor);

    if (bestEval >= beta)
    {
        ++_cutoffNodes;
        return bestEval;
    }
    alpha = std::max(alpha, bestEval);

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

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

int BestMoveSearch::_zwQuiescenceSearch(Board& bd, const int alpha, uint64_t zHash)
{
    const int beta = alpha + 1;

    ++_visitedNodes;
    int bestEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.movColor);

    if (bestEval >= beta)
    {
        ++_cutoffNodes;
        return bestEval;
    }

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        _fetchBestMove(moves, i);

        Move::MakeMove(moves[i], bd);
        const int moveValue = -_zwQuiescenceSearch( bd, -beta, zHash);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

        if (moveValue > bestEval)
        {
            bestEval = moveValue;
            if (moveValue >= beta)
            {
                ++ _cutoffNodes;
                break;
            }

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
    assert(mv.IsOkeyMove());

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
        std::swap(moves.data[ind], moves.data[0]);

    assert(ind != moves.size);
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
    assert(maxInd != targetPos || moves.data[targetPos].GetEval() >= moves.data[targetPos+1].GetEval());
}

int BestMoveSearch::_getMateValue(const int depthLeft) const
{
    const int distToRoot = _currRootDepth - depthLeft;
    return NegativeInfinity + distToRoot;
}
