//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/Search/BestMoveSearch.h"

#include <cassert>
#include <chrono>
#include <format>
#include <string>
#include <vector>

#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/Search/TranspositionTable.h"
#include "../include/Search/ZobristHash.h"

static constexpr int NO_EVAL =  TranspositionTable::HashRecord::NoEval;

void BestMoveSearch::IterativeDeepening(PackedMove* output, const int maxDepth, const bool writeInfo)
{
    const uint64_t zHash = ZHasher.GenerateHash(_board);
    int eval{};
    long avg{};

    if (maxDepth == 0)
    {
        GlobalLogger.StartLogging() << "info depth 0 score cp "
            << BoardEvaluator::DefaultFullEvalFunction(_board, _board.movColor) << std::endl;
        return;
    }

    for (int depth = 1; depth < maxDepth; ++depth)
    {
        PV pv{depth};
        PV pvBuff{depth};
        // measuring time
        [[maybe_unused]]auto t1 = std::chrono::steady_clock::now();

        // preparing variables used to display statistics
        _currRootDepth = depth;
        _visitedNodes = 0;
        _cutoffNodes = 0;

        // cleaning tables used in iteration

        if (depth < 7)
        {
            _kTable.ClearPlyFloor(depth);
            _histTable.ScaleTableDown();
            eval = _pwsSearch(_board, NegativeInfinity, PositiveInfinity, depth, zHash, {}, pv, true);
            avg += eval;
        }
        else
        {
            const int averageScore = avg / (depth - 1);
            int delta = BoardEvaluator::BasicFigureValues[wPawnsIndex] / 16;
            int alpha = averageScore - delta;
            int beta = averageScore + delta;

            int tries = 0;
            while (true)
            {
                delta += delta;
                tries++;
                pvBuff.Clone(pv);
                _kTable.ClearPlyFloor(depth);
                _histTable.ScaleTableDown();
                eval = _pwsSearch(_board, alpha, beta, depth, zHash, {}, pvBuff, true);

                if (eval <= alpha)
                {
                    beta = (alpha + beta) / 2;
                    alpha = std::max(alpha - delta, NegativeInfinity);
                }
                else if (eval >= beta)
                    beta = std::min(beta + delta, PositiveInfinity);
                else
                    break;
            }

            GlobalLogger.StartLogging() << std::format("[ WARN ] Total aspiration tries: {}\n", tries);
            pv.Clone(pvBuff);
            avg += eval;
        }

        // measurement end
        [[maybe_unused]]auto t2 = std::chrono::steady_clock::now();

        // saving move
        *output = pv[0];

        if (writeInfo)
        {
            static constexpr uint64_t MSEC = 1000 * 1000; // in nsecs
            const uint64_t spentMs = std::max(1LU, (t2-t1).count()/MSEC);
            const uint64_t nps = 1000LLU * _visitedNodes / spentMs;
            const double cutOffPerc = static_cast<double>(_cutoffNodes)/static_cast<double>(_visitedNodes);

            GlobalLogger.StartLogging() << std::format("info depth {} time {} nodes {} nps {} score cp {} currmove {} hashfull {} cut-offs perc {:.2f} pv ",
                                                       depth + 1, spentMs, _visitedNodes, nps, eval*BoardEvaluator::ScoreGrain,  output->GetLongAlgebraicNotation(), TTable.GetContainedElements(), cutOffPerc);

            pv.Print();
            GlobalLogger.StartLogging() << std::endl;
        }
    }
}

int BestMoveSearch::_pwsSearch(Board& bd, int alpha, const int beta, const int depthLeft, uint64_t zHash, const Move prevMove, PV& pv, bool followPv)
{
    assert(alpha < beta);

    nodeType nType = upperBound;
    PackedMove bestMove{};

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _quiescenceSearch(bd, alpha, beta, zHash);

    // incrementing nodes counter;
    ++_visitedNodes;

    // generate moves
    MoveGenerator mechanics(bd, _stack, _histTable, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField());
    auto moves = mechanics.GetMovesFast();

    if (moves.size == 0)
        return mechanics.IsCheck() ? _getMateValue(depthLeft) : 0;

    // reading Transposition table for the best move
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if (wasTTHit && prevSearchRes.GetNodeType() != upperBound)
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else if (!followPv || depthLeft == 1)
        _fetchBestMove(moves, 0);
    else _pullMoveToFront(moves, pv(depthLeft, _currRootDepth));

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    PV inPV{depthLeft};

    zHash = ZHasher.UpdateHash(zHash, moves[0], oldElPassant, oldCastlings);
    TTable.Prefetch(zHash);
    Move::MakeMove(moves[0], bd);
    _kTable.ClearPlyFloor(depthLeft - 1);
    int bestEval = -_pwsSearch(bd, -beta, -alpha, depthLeft - 1, zHash, moves[0], inPV, followPv);
    zHash = ZHasher.UpdateHash(zHash, moves[0], oldElPassant, oldCastlings);
    Move::UnmakeMove(moves[0], bd, oldCastlings, oldElPassant);

    if (bestEval >= alpha)
    {
        // cut-off found
        if (bestEval >= beta)
        {
            ++_cutoffNodes;

            if (moves[0].IsQuietMove())
            {
                _kTable.SaveKillerMove(moves[0], depthLeft);
                _cmTable.SaveCounterMove(moves[0].GetPackedMove(), prevMove);
                _histTable.SetBonusMove(moves[0], depthLeft);
            }

            // updating if profitable
            if (depthLeft >= prevSearchRes.GetDepth()
                || (!wasTTHit && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
            {
                const TranspositionTable::HashRecord record{zHash, moves[0].GetPackedMove(),
                    bestEval, wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL, depthLeft, lowerBound, _age };
                TTable.Add(record, zHash);
            }

            _stack.PopAggregate(moves);
            return bestEval;
        }

        nType = pvNode;
        alpha = bestEval;
        bestMove = moves[0].GetPackedMove();
        pv.InsertNext(bestMove,inPV);
    }

    // processsing each move
    for (size_t i = 1; i < moves.size; ++i)
    {
        _fetchBestMove(moves, i);

        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        TTable.Prefetch(zHash);
        Move::MakeMove(moves[i], bd);

        // performing checks wether assumed thesis holds
        _kTable.ClearPlyFloor(depthLeft - 1);
        int moveEval = -_zwSearch(bd, -alpha -1, depthLeft - 1, zHash, moves[i]);

        // if not, research move
        if (alpha < moveEval && moveEval < beta)
        {
            TTable.Prefetch(zHash);
            _kTable.ClearPlyFloor(depthLeft - 1);
            moveEval = -_pwsSearch(bd, -beta, -alpha, depthLeft - 1, zHash, moves[i], inPV, false);

            if (moveEval > bestEval)
            {
                bestEval = moveEval;

                if (moveEval >= alpha)
                {
                    bestMove = moves[i].GetPackedMove();

                    // cut-off found
                    if (moveEval >= beta)
                    {
                        if (moves[i].IsQuietMove())
                        {
                            _kTable.SaveKillerMove(moves[i], depthLeft);
                            _cmTable.SaveCounterMove(moves[i].GetPackedMove(), prevMove);
                            _histTable.SetBonusMove(moves[i], depthLeft);
                        }
                        nType = lowerBound;

                        ++_cutoffNodes;
                        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
                        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);
                        break;
                    }

                    nType = pvNode;
                    pv.InsertNext(bestMove,inPV);
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
        || (!wasTTHit && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval,
            wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL, depthLeft, nType, _age };
        TTable.Add(record, zHash);
    }

    assert(nType != pvNode || bestMove.IsOkeyMove());

    return bestEval;
}

[[nodiscard]] int BestMoveSearch::_zwSearch(Board& bd, const int alpha, const int depthLeft, uint64_t zHash, const Move prevMove) {
    const int beta = alpha + 1;
    int bestEval = NegativeInfinity;
    nodeType nType = upperBound;
    PackedMove bestMove {};

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _zwQuiescenceSearch(bd, alpha, zHash);

    // incrementing nodes counter;
    ++_visitedNodes;

    // reading Transposition table for previous score
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if (wasTTHit && prevSearchRes.GetDepth() >= depthLeft)
    {
        const nodeType expectedType = prevSearchRes.GetEval() >= beta ? lowerBound : upperBound;

        if (expectedType == prevSearchRes.GetNodeType() || prevSearchRes.GetNodeType() == pvNode)
        {
            ++_cutoffNodes;
            return prevSearchRes.GetEval();
        }
    }

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    // generate moves
    MoveGenerator mechanics(bd, _stack, _histTable, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField());
    auto moves = mechanics.GetMovesFast();

    if (moves.size == 0)
        return mechanics.IsCheck() ? _getMateValue(depthLeft) : 0;

    // processing each move
    for (size_t i = 0; i < moves.size; ++i)
    {
        // load the best move from TT if possible
        if (i == 0 && prevSearchRes.IsSameHash(zHash) && prevSearchRes.GetNodeType() != upperBound)
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
                if (moves[i].IsQuietMove())
                {
                    _kTable.SaveKillerMove(moves[i], depthLeft);
                    _cmTable.SaveCounterMove(moves[i].GetPackedMove(), prevMove);
                    _histTable.SetBonusMove(moves[i], depthLeft);
                }

                nType = lowerBound;
                bestMove = moves[i].GetPackedMove();
                ++_cutoffNodes;
                break;
            }
        }
    }

    if (depthLeft >= prevSearchRes.GetDepth()
            || (!wasTTHit && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval,
            wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL, depthLeft, nType, _age };
        TTable.Add(record, zHash);
    }

    //clean up
    _stack.PopAggregate(moves);
    return bestEval;
}

int BestMoveSearch::_quiescenceSearch(Board& bd, int alpha, const int beta, uint64_t zHash)
{
    assert(beta > alpha);

    int statEval;
    PackedMove bestMove{};
    nodeType nType = upperBound;
    ++_visitedNodes;

    // reading Transposition table for the best move
    auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if (wasTTHit && prevSearchRes.GetStatVal() != NO_EVAL)
        statEval = prevSearchRes.GetStatVal();
    else
        statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.movColor);

    int bestEval = statEval;

    if (bestEval >= alpha)
    {
        if (bestEval >= beta)
        {
            ++_cutoffNodes;

            if (wasTTHit && prevSearchRes.GetStatVal() == NO_EVAL)
            {
                prevSearchRes.SetStatVal(statEval);
                TTable.Add(prevSearchRes, zHash);
            }
            return bestEval;
        }

        alpha = bestEval;
    }

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    if (wasTTHit && prevSearchRes.GetNodeType() != upperBound && prevSearchRes.GetMove().IsCapture())
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i != 0)
            _fetchBestMove(moves, i);

        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        TTable.Prefetch(zHash);
        Move::MakeMove(moves[i], bd);
        const int moveValue = -_quiescenceSearch( bd, -beta, -alpha, zHash);
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);

        if (moveValue > bestEval)
        {
            bestEval = moveValue;

            if (moveValue >= alpha)
            {
                bestMove = moves[i].GetPackedMove();
                if (moveValue >= beta)
                {
                    ++ _cutoffNodes;

                    nType = lowerBound;
                    break;
                }

                alpha = moveValue;
            }
        }
    }

    // clean up
    _stack.PopAggregate(moves);

    if (prevSearchRes.GetDepth() == 0 ||
        (wasTTHit == false && _age - prevSearchRes.GetAge() >= QuisenceAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statEval, 0, nType, _age };
        TTable.Add(record, zHash);
    }
    return bestEval;
}

int BestMoveSearch::_zwQuiescenceSearch(Board& bd, const int alpha, uint64_t zHash)
{
    const int beta = alpha + 1;

    int statEval;
    nodeType nType = upperBound;
    PackedMove bestMove{};

    ++_visitedNodes;

    // reading Transposition table for the best move
    auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if (wasTTHit)
    {
        const nodeType expectedType = prevSearchRes.GetEval() >= beta ? lowerBound : upperBound;

        if (expectedType == prevSearchRes.GetNodeType() || prevSearchRes.GetNodeType() == pvNode)
        {
            ++_cutoffNodes;
            return prevSearchRes.GetEval();
        }

        if (prevSearchRes.GetStatVal() != NO_EVAL)
            statEval = prevSearchRes.GetStatVal();
        else
            statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.movColor);
    }
    else
        statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.movColor);

    int bestEval = statEval;
    if (bestEval >= beta)
    {
        ++_cutoffNodes;

        if (wasTTHit && prevSearchRes.GetStatVal() == NO_EVAL)
        {
            prevSearchRes.SetStatVal(statEval);
            TTable.Add(prevSearchRes, zHash);
        }

        return bestEval;
    }

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.elPassantField;

    if (wasTTHit && prevSearchRes.GetNodeType() != upperBound && prevSearchRes.GetMove().IsCapture())
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else _fetchBestMove(moves, 0);

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i != 0)
            _fetchBestMove(moves, i);

        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        TTable.Prefetch(zHash);
        Move::MakeMove(moves[i], bd);
        const int moveValue = -_zwQuiescenceSearch( bd, -beta, zHash);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);

        if (moveValue > bestEval)
        {
            bestEval = moveValue;
            if (moveValue >= beta)
            {
                ++ _cutoffNodes;

                nType = lowerBound;
                bestMove = moves[i].GetPackedMove();
                break;
            }

        }
    }

    if (prevSearchRes.GetDepth() == 0
        || (wasTTHit == false && _age - prevSearchRes.GetAge() >= QuisenceAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statEval, 0, nType, _age };
        TTable.Add(record, zHash);
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

void BestMoveSearch::_pullMoveToFront(MoveGenerator::payload moves, const PackedMove mv)
{
    assert(mv.IsOkeyMove());

    // preparing sentinel
    const Move sentinelOld = moves.data[moves.size];
    moves.data[moves.size] = Move(mv);

    // finding stopping index
    size_t ind = 0;
    while(moves.data[ind] != Move(mv)) ind++;

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
    assert(maxInd == targetPos || moves.data[targetPos].GetEval() >= moves.data[targetPos+1].GetEval());
}

int BestMoveSearch::_getMateValue(const int depthLeft) const
{
    const int distToRoot = _currRootDepth - depthLeft;
    return NegativeInfinity + distToRoot;
}
