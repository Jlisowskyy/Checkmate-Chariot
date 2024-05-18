//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/Search/BestMoveSearch.h"

#include <chrono>
#include <format>
#include <vector>

#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/Search/TranspositionTable.h"
#include "../include/Search/ZobristHash.h"
#include "../include/ThreadManagement/GameTimeManager.h"

static constexpr int NO_EVAL = TranspositionTable::HashRecord::NoEval;

#ifndef NDEBUG
#define TestNullMove() TraceIfFalse(!record._madeMove.IsEmpty(), "Received empty move inside the TT")
#define TestTTAdd()                                                                                                    \
    TraceIfFalse(record._type != nodeType::upperBound, "Received upper board node inside the TT!");                    \
    TestNullMove()
#else
#define TestNullMove()
#define TestTTAdd()
#endif

void BestMoveSearch::IterativeDeepening(
    PackedMove *bestMove, PackedMove *ponderMove, const int32_t maxDepth, const bool writeInfo
)
{
    // When the passed depth is 0, we need to evaluate the board statically
    if (maxDepth == 0)
    {
        GlobalLogger.LogStream << "info depth 0 score cp "
                               << BoardEvaluator::DefaultFullEvalFunction(_board, _board.MovingColor) << std::endl;

        return;
    }

    // Generate unique hash for the board
    const uint64_t zHash = ZHasher.GenerateHash(_board);
    int32_t eval{};
    int64_t avg{};

    // prepare pv buffer
    PV pvBuff{};

    // usual search path
    for (int32_t depth = 1; depth <= maxDepth; ++depth)
    {
        // Search start time point
        [[maybe_unused]] auto timeStart = GameTimeManager::GetCurrentTime();

        // preparing variables used to display statistics
        _currRootDepth = depth;
        _visitedNodes  = 0;
        _cutoffNodes   = 0;

        if (depth < 5)
        {
            // set according depth inside the pv buffer
            _pv.SetDepth(depth);

            // cleaning tables used in previous iteration
            _kTable.ClearPlyFloor(depth);
            _histTable.ScaleTableDown();

            // performs the search without aspiration window to gather some initial statistics about the move
            eval = _pwsSearch(_board, NegativeInfinity, PositiveInfinity, depth, zHash, {}, _pv, true);
            TraceIfFalse(_pv.IsFilled(), "PV buffer is not filled after the search!");

            // if there was call to abort then abort
            if (std::abs(eval) == TimeStopValue)
                return;

            // saving the move evaluation to the avg value
            // TODO: maybe check previous pv for the predicting value?
            avg += depth * eval;
        }
        else
        {
            // Preparing variables for the aspiration window framework
            const int64_t coefSum   = (depth + 1) * depth / 2;
            const auto averageScore = static_cast<int32_t>(avg / coefSum);
            int32_t delta           = InitialAspWindowDelta;
            int32_t alpha           = averageScore - delta;
            int32_t beta            = averageScore + delta;
            pvBuff.SetDepth(depth);

            // TODO: add some kind of logging inside the debug mode
            // Aspiration window loop
            while (true)
            {
                delta += delta;

                // cleaning tables used in previous iterations
                _kTable.ClearPlyFloor(depth);
                _histTable.ScaleTableDown();

                eval = _pwsSearch(_board, alpha, beta, depth, zHash, {}, pvBuff, true);

                // if there was call to abort then abort
                if (std::abs(eval) == TimeStopValue)
                    return;

                if (eval <= alpha)
                {
                    // TODO: WTF?
                    beta  = (alpha + beta) / 2;
                    alpha = std::max(alpha - delta, NegativeInfinity);
                }
                else if (eval >= beta)
                    // We failed high so move the upper boundary
                    beta = std::min(beta + delta, PositiveInfinity);
                else
                    break;
            }

            // Move the pv from the buffer to the main pv
            _pv.Clone(pvBuff);

            TraceIfFalse(_pv.IsFilled(), "PV buffer is not filled after the search!");

            // Update avg cumulating variable
            avg += depth * eval;
        }

        // Search stop time point
        [[maybe_unused]] auto timeStop = GameTimeManager::GetCurrentTime();

        // Saving first move from the PV as the best move

        if (bestMove != nullptr)
            *bestMove = _pv[0];

        if (ponderMove != nullptr && depth > 1)
            *ponderMove = _pv[1];

        // Log info if necessary
        if (writeInfo)
        {
            const uint64_t spentMs  = std::max(static_cast<uint64_t>(1), (timeStop - timeStart).count() / MsesInNsec);
            const uint64_t nps      = 1000LLU * _visitedNodes / spentMs;
            const double cutOffPerc = static_cast<double>(_cutoffNodes) / static_cast<double>(_visitedNodes);

            GlobalLogger.LogStream << std::format(
                "info depth {} time {} nodes {} nps {} score cp {} currmove {} hashfull {} cut-offs perc {:.2f} pv ",
                depth, spentMs, _visitedNodes, nps, eval * BoardEvaluator::ScoreGrain,
                bestMove->GetLongAlgebraicNotation(), TTable.GetContainedElements(), cutOffPerc
            );

            _pv.Print();
            GlobalLogger.LogStream << std::endl;
        }
    }
}

int BestMoveSearch::_pwsSearch(
    Board &bd, int alpha, const int beta, const int depthLeft, uint64_t zHash, const Move prevMove, PV &pv,
    bool followPv
)
{
    TraceIfFalse(alpha < beta, "Alpha is not less than beta");

    nodeType nType = upperBound;
    PackedMove bestMove{};

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TimeStopValue;

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _quiescenceSearch(bd, alpha, beta, zHash);

    // incrementing nodes counter;
    ++_visitedNodes;

    // generate moves
    MoveGenerator mechanics(
        bd, _stack, _histTable, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField()
    );
    auto moves = mechanics.GetMovesFast();

    if (moves.size == 0)
        return mechanics.IsCheck() ? _getMateValue(depthLeft) : 0;

    // reading Transposition table for the best move
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if constexpr (TestTT) TTable.UpdateStatistics(wasTTHit);

    if (followPv && depthLeft != 1)
        _pullMoveToFront(moves, _pv(depthLeft, _currRootDepth));
    else if (wasTTHit && prevSearchRes.GetNodeType() != upperBound)
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // saving old params
    const auto oldCastlings = bd.Castlings;
    const auto oldElPassant = bd.ElPassantField;

    PV inPV{depthLeft};

    zHash = ZHasher.UpdateHash(zHash, moves[0], oldElPassant, oldCastlings);
    TTable.Prefetch(zHash);
    Move::MakeMove(moves[0], bd);
    _kTable.ClearPlyFloor(depthLeft - 1);
    int bestEval = -_pwsSearch(bd, -beta, -alpha, depthLeft - 1, zHash, moves[0], inPV, followPv);

    // if there was call to abort then abort
    if (std::abs(bestEval) == TimeStopValue)
        return TimeStopValue;

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
            if (depthLeft >= prevSearchRes.GetDepth() ||
                (!wasTTHit && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
            {
                const TranspositionTable::HashRecord record{zHash,     moves[0].GetPackedMove(),
                                                            bestEval,  wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL,
                                                            depthLeft, lowerBound,
                                                            _age};
                TTable.Add(record, zHash);
                TestTTAdd();
            }

            _stack.PopAggregate(moves);
            return bestEval;
        }

        nType    = pvNode;
        alpha    = bestEval;
        bestMove = moves[0].GetPackedMove();
        pv.InsertNext(bestMove, inPV);
    }

    // processing each move
    for (size_t i = 1; i < moves.size; ++i)
    {
        _fetchBestMove(moves, i);

        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        TTable.Prefetch(zHash);
        Move::MakeMove(moves[i], bd);

        // performing checks whether assumed thesis holds
        _kTable.ClearPlyFloor(depthLeft - 1);
        int moveEval = -_zwSearch(bd, -alpha - 1, depthLeft - 1, zHash, moves[i]);

        // if there was call to abort then abort
        if (std::abs(moveEval) == TimeStopValue)
            return TimeStopValue;

        // if not, research move
        if (alpha < moveEval && moveEval < beta)
        {
            TTable.Prefetch(zHash);
            _kTable.ClearPlyFloor(depthLeft - 1);
            moveEval = -_pwsSearch(bd, -beta, -alpha, depthLeft - 1, zHash, moves[i], inPV, false);

            // if there was call to abort then abort
            if (std::abs(moveEval) == TimeStopValue)
                return TimeStopValue;

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
                    pv.InsertNext(bestMove, inPV);
                }
            }
        }
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);
        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);
    }

    // clean up
    _stack.PopAggregate(moves);

    // updating if profitable
    if (depthLeft >= prevSearchRes.GetDepth() || (!wasTTHit && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{
            zHash, bestMove, bestEval, wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL, depthLeft, nType, _age
        };
        TTable.Add(record, zHash);
    }

    TraceIfFalse(
        nType != pvNode || bestMove.IsOkeyMove(),
        "When the node fails low there should be no best move (null move), otherwise we expect the node to be PV one"
    );

    return bestEval;
}

[[nodiscard]] int
BestMoveSearch::_zwSearch(Board &bd, const int alpha, const int depthLeft, uint64_t zHash, const Move prevMove)
{
    const int beta = alpha + 1;
    int bestEval   = NegativeInfinity;
    nodeType nType = upperBound;
    PackedMove bestMove{};

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TimeStopValue;

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _zwQuiescenceSearch(bd, alpha, zHash);

    // incrementing nodes counter;
    ++_visitedNodes;

    // reading Transposition table for previous score
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if constexpr (TestTT) TTable.UpdateStatistics(wasTTHit);

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
    const auto oldElPassant = bd.ElPassantField;

    // generate moves
    MoveGenerator mechanics(
        bd, _stack, _histTable, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField()
    );
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

        // if there was call to abort then abort
        if (std::abs(moveEval) == TimeStopValue)
            return TimeStopValue;

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

                nType    = lowerBound;
                bestMove = moves[i].GetPackedMove();
                ++_cutoffNodes;
                break;
            }
        }
    }

    if (depthLeft >= prevSearchRes.GetDepth() || (!wasTTHit && _age - prevSearchRes.GetAge() >= SearchAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{
            zHash, bestMove, bestEval, wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL, depthLeft, nType, _age
        };
        TTable.Add(record, zHash);
    }

    // clean up
    _stack.PopAggregate(moves);
    return bestEval;
}

int BestMoveSearch::_quiescenceSearch(Board &bd, int alpha, const int beta, uint64_t zHash)
{
    TraceIfFalse(beta > alpha, "Beta is not greater than alpha");

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TimeStopValue;

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
        statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);

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
    const auto oldElPassant = bd.ElPassantField;

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
        const int moveValue = -_quiescenceSearch(bd, -beta, -alpha, zHash);

        // if there was call to abort then abort
        if (std::abs(moveValue) == TimeStopValue)
            return TimeStopValue;

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
                    ++_cutoffNodes;

                    nType = lowerBound;
                    break;
                }

                alpha = moveValue;
            }
        }
    }

    // clean up
    _stack.PopAggregate(moves);

    if (prevSearchRes.GetDepth() == 0 || (!wasTTHit && _age - prevSearchRes.GetAge() >= QuisenceAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statEval, 0, nType, _age};
        TTable.Add(record, zHash);
    }
    return bestEval;
}

int BestMoveSearch::_zwQuiescenceSearch(Board &bd, const int alpha, uint64_t zHash)
{
    const int beta = alpha + 1;

    int statEval;
    nodeType nType = upperBound;
    PackedMove bestMove{};

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TimeStopValue;

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
            statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);
    }
    else
        statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);

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
    const auto oldElPassant = bd.ElPassantField;

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
        const int moveValue = -_zwQuiescenceSearch(bd, -beta, zHash);

        // if there was call to abort then abort
        if (std::abs(moveValue) == TimeStopValue)
            return TimeStopValue;

        Move::UnmakeMove(moves[i], bd, oldCastlings, oldElPassant);
        zHash = ZHasher.UpdateHash(zHash, moves[i], oldElPassant, oldCastlings);

        if (moveValue > bestEval)
        {
            bestEval = moveValue;
            if (moveValue >= beta)
            {
                ++_cutoffNodes;

                nType    = lowerBound;
                bestMove = moves[i].GetPackedMove();
                break;
            }
        }
    }

    if (prevSearchRes.GetDepth() == 0 || (!wasTTHit && _age - prevSearchRes.GetAge() >= QuisenceAgeDiffToReplace))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statEval, 0, nType, _age};
        TTable.Add(record, zHash);
    }

    // clean up
    _stack.PopAggregate(moves);
    return bestEval;
}

void BestMoveSearch::_embeddedMoveSort(MoveGenerator::payload moves, const size_t range)
{
    for (signed_size_t i = 1; i < static_cast<signed_size_t>(range); ++i)
    {
        signed_size_t j = i - 1;
        const auto val  = moves[i];
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
    TraceIfFalse(mv.IsOkeyMove(), "Given move to find is a null move!");

    // preparing sentinel
    const Move sentinelOld = moves.data[moves.size];
    moves.data[moves.size] = Move(mv);

    // finding stopping index
    size_t ind = 0;
    while (moves.data[ind] != Move(mv)) ind++;

    // replacing old element
    moves.data[moves.size] = sentinelOld;

    // if move found swapping
    if (ind != moves.size)
    {
        std::swap(moves.data[ind], moves.data[0]);
        return;
    }

    WrapTraceMsgError("Move stored inside the TT was not found in the moves list!");
}

void BestMoveSearch::_fetchBestMove(MoveGenerator::payload moves, const size_t targetPos)
{
    int maxValue  = NegativeInfinity;
    size_t maxInd = targetPos;

    for (size_t i = targetPos; i < moves.size; ++i)
    {
        if (const int heuristicEval = moves[i].GetEval(); heuristicEval > maxValue)
        {
            maxInd   = i;
            maxValue = heuristicEval;
        }
    }

    std::swap(moves.data[maxInd], moves.data[targetPos]);
    TraceIfFalse(
        maxInd == targetPos || moves.data[targetPos].GetEval() >= moves.data[targetPos + 1].GetEval(),
        "Move sorting failed!"
    );
}

int BestMoveSearch::_getMateValue(const int depthLeft) const
{
    const int distToRoot = _currRootDepth - depthLeft;
    return NegativeInfinity + distToRoot;
}
