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
#include "../include/TestsAndDebugging/DebugTools.h"
#include "../include/ThreadManagement/GameTimeManager.h"

static constexpr int NO_EVAL = TranspositionTable::HashRecord::NoEval;
using RepMap = std::map<uint64_t, int>;

#ifndef NDEBUG

#define TestNullMove() TraceIfFalse(!record._madeMove.IsEmpty(), "Received empty move inside the TT")
#define TestTTAdd()                                                                                                    \
    TraceIfFalse(record._type != nodeType::upperBound, "Received upper board node inside the TT!");                    \
    TestNullMove()

#else
#define TestNullMove()
#define TestTTAdd()
#endif // NDEBUG

[[nodiscard]] constexpr INLINE uint64_t ProcessAttackMove(Board& bd, const Move mv, const uint64_t hash, const VolatileBoardData& data, RepMap& rMap)
{

    const uint64_t nextHash = ZHasher.UpdateHash(hash, mv, data);
    TTable.Prefetch(nextHash);
    Move::MakeMove(mv, bd);
    rMap[nextHash]++;

    return nextHash;
}

[[nodiscard]] constexpr INLINE uint64_t ProcessMove(Board& bd, const Move mv, const int depth, const uint64_t hash, KillerTable& table, const VolatileBoardData& data, RepMap& rMap)
{

    const uint64_t nextHash = ZHasher.UpdateHash(hash, mv, data);
    TTable.Prefetch(nextHash);
    Move::MakeMove(mv, bd);
    table.ClearPlyFloor(depth);
    rMap[nextHash]++;

    return nextHash;
}

[[nodiscard]] INLINE uint64_t RevertMove(Board& bd, const Move mv, const uint64_t hash, const VolatileBoardData& data, RepMap& rMap)
{
    Move::UnmakeMove(mv, bd, data);

    if (const int occurs = --rMap[hash]; occurs == 0)
        rMap.erase(hash);

    return ZHasher.UpdateHash(hash, mv, data);
}

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

        if (true)
        {
            // set according depth inside the pv buffer
            _pv.SetDepth(depth);

            // cleaning tables used in previous iteration
            _kTable.ClearPlyFloor(depth);
            _histTable.ScaleTableDown();

            // performs the search without aspiration window to gather some initial statistics about the move
            eval = _pwsSearch(_board, NEGATIVE_INFINITY, POSITIVE_INFINITY, depth, zHash, {}, _pv, true);
            TraceIfFalse(_pv.IsFilled(), "PV buffer is not filled after the search!");

            // if there was call to abort then abort
            if (std::abs(eval) == TIME_STOP_RESERVED_VALUE)
                break;

            // saving the move evaluation to the avg value
            // TODO: maybe check previous pv for the predicting value?
            avg += depth * eval;
        }
        else
        {
            // Preparing variables for the aspiration window framework
            const int64_t coefSum   = (depth + 1) * depth / 2;
            const auto averageScore = static_cast<int32_t>(avg / coefSum);
            int32_t delta           = INITIAL_ASP_WINDOW_DELTA;
            int32_t alpha           = averageScore - delta;
            int32_t beta            = averageScore + delta;
            pvBuff.SetDepth(depth);
            [[maybe_unused]] AspWinStat stat{}; // Used only with TEST_ASP_WIN define

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
                if (std::abs(eval) == TIME_STOP_RESERVED_VALUE)
                    break;

                if (eval <= alpha)
                {
                    if constexpr (TestAsp)
                        stat.RetryFailLow(alpha, beta, eval);

                    alpha = std::max(alpha - delta, NEGATIVE_INFINITY);
                    beta  = (alpha + beta) / 2;
                }
                else if (eval >= beta)
                {
                    if constexpr (TestAsp)
                        stat.RetryFailHigh(alpha, beta, eval);

                    // We failed high so move the upper boundary
                    beta = std::min(beta + delta, POSITIVE_INFINITY);
                }
                else
                    break;
            }

            // Display Asp Window statistics
            if constexpr (TestAsp)
                stat.DisplayAndClean();

            // if there was call to abort then abort
            if (std::abs(eval) == TIME_STOP_RESERVED_VALUE)
                break;

            // Move the pv from the buffer to the main pv
            _pv.Clone(pvBuff);

            TraceIfFalse(_pv.IsFilled(), "PV buffer is not filled after the search!");

            // Update avg cumulating variable
            avg += depth * eval;
        }

        // if there was call to abort then abort
        if (std::abs(eval) == TIME_STOP_RESERVED_VALUE)
            break;

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
            const uint64_t spentMs  = std::max(static_cast<uint64_t>(1), (timeStop - timeStart).count() / MSEC_TO_NSEC);
            const uint64_t nps      = 1000LLU * _visitedNodes / spentMs;
            const double cutOffPerc = static_cast<double>(_cutoffNodes) / static_cast<double>(_visitedNodes);

            GlobalLogger.LogStream << std::format(
                "info depth {} time {} nodes {} nps {} score cp {} currmove {} hashfull {} cut-offs perc {:.2f} pv ",
                depth, spentMs, _visitedNodes, nps, eval * BoardEvaluator::ScoreGrain,
                _pv[0].GetLongAlgebraicNotation(), TTable.GetContainedElements(), cutOffPerc
            );

            _pv.Print();
            GlobalLogger.LogStream << std::endl;
        }
    }

    if constexpr (TestTT)
        TTable.DisplayStatisticsAndReset();
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
        return TIME_STOP_RESERVED_VALUE;

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

    // If no move is possible: check whether we hit mate or stalemate
    if (moves.size == 0)
        return mechanics.IsCheck() ? _getMateValue(depthLeft) : 0;

    // reading Transposition table for the best move
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // check whether hashes are same
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if constexpr (TestTT)
        TTable.UpdateStatistics(wasTTHit);

    if (followPv && depthLeft != 1)
        // first follow pv from previous ID (Iterative deepening) iteration,
        // we follow only single PV we previously saved
        _pullMoveToFront(moves, _pv(depthLeft, _currRootDepth));
    else if (wasTTHit && !prevSearchRes.GetMove().IsEmpty())
        // if we have any move saved from last time we visited that node and the move is valid try to use it
        // NOTE: We don't store moves from fail low nodes only score so the move from fail low should always be empty
        //       In other words we don't use best move from fail low nodes
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // saving volatile board state
    VolatileBoardData oldData{bd};

    PV inPV{depthLeft};

    zHash = ProcessMove(bd, moves[0], depthLeft-1, zHash, _kTable, oldData, _repMap);
    int bestEval = -_pwsSearch(bd, -beta, -alpha, depthLeft - 1, zHash, moves[0], inPV, followPv);
    zHash = RevertMove(bd, moves[0], zHash, oldData, _repMap);

    // if there was call to abort then abort
    if (std::abs(bestEval) == TIME_STOP_RESERVED_VALUE)
        return TIME_STOP_RESERVED_VALUE;

    if (bestEval >= alpha)
    {
        // cut-off found
        if (bestEval >= beta)
        {
            ++_cutoffNodes;

            if (moves[0].IsQuietMove())
                _saveQuietMoveInfo(moves[0], prevMove, depthLeft);

            // updating if profitable
            if (depthLeft >= prevSearchRes.GetDepth() ||
                (!wasTTHit && _age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE))
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

        zHash = ProcessMove(bd, moves[i], depthLeft-1, zHash, _kTable, oldData, _repMap);
        int moveEval = -_zwSearch(bd, -alpha - 1, depthLeft - 1, zHash, moves[i]);

        // if there was call to abort then abort
        if (std::abs(moveEval) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

        // if not, research move
        if (alpha < moveEval && moveEval < beta)
        {
            TTable.Prefetch(zHash);
            _kTable.ClearPlyFloor(depthLeft - 1);
            moveEval = -_pwsSearch(bd, -beta, -alpha, depthLeft - 1, zHash, moves[i], inPV, false);

            // if there was call to abort then abort
            if (std::abs(moveEval) == TIME_STOP_RESERVED_VALUE)
                return TIME_STOP_RESERVED_VALUE;

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
                            _saveQuietMoveInfo(moves[i], prevMove, depthLeft);
                        nType = lowerBound;

                        ++_cutoffNodes;
                        zHash = RevertMove(bd, moves[i], zHash, oldData, _repMap);
                        break;
                    }

                    nType = pvNode;
                    pv.InsertNext(bestMove, inPV);
                }
            }
        }

        // move reverted after possible research
        zHash = RevertMove(bd, moves[i], zHash, oldData, _repMap);
    }

    // clean up
    _stack.PopAggregate(moves);

    // updating if profitable
    if (depthLeft >= prevSearchRes.GetDepth() || (!wasTTHit && _age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE))
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
    int bestEval   = NEGATIVE_INFINITY;
    nodeType nType = upperBound;
    PackedMove bestMove{};

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _zwQuiescenceSearch(bd, alpha, zHash);

    // incrementing nodes counter;
    ++_visitedNodes;

    // reading Transposition table for previous score
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if constexpr (TestTT)
        TTable.UpdateStatistics(wasTTHit);

    if (wasTTHit && prevSearchRes.GetDepth() >= depthLeft)
    {
        const nodeType expectedType = prevSearchRes.GetEval() >= beta ? lowerBound : upperBound;

        if (expectedType == prevSearchRes.GetNodeType() || prevSearchRes.GetNodeType() == pvNode)
        {
            ++_cutoffNodes;
            return prevSearchRes.GetEval();
        }
    }

    // saving volatile fields
    VolatileBoardData oldData{bd};

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
        if (i == 0 && wasTTHit && !prevSearchRes.GetMove().IsEmpty())
            _pullMoveToFront(moves, prevSearchRes.GetMove());
        else
            _fetchBestMove(moves, i);

        zHash = ProcessMove(bd, moves[i], depthLeft-1, zHash, _kTable, oldData, _repMap);
        const int moveEval = -_zwSearch(bd, -beta, depthLeft - 1, zHash, moves[i]);
        zHash = RevertMove(bd, moves[i], zHash, oldData, _repMap);

        // if there was call to abort then abort
        if (std::abs(moveEval) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

        if (moveEval > bestEval)
        {
            bestEval = moveEval;

            // cut-off found
            if (moveEval >= beta)
            {
                if (moves[i].IsQuietMove())
                    _saveQuietMoveInfo(moves[i], prevMove, depthLeft);

                nType    = lowerBound;
                bestMove = moves[i].GetPackedMove();
                ++_cutoffNodes;
                break;
            }
        }
    }

    if (depthLeft >= prevSearchRes.GetDepth() || (!wasTTHit && _age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE))
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
        return TIME_STOP_RESERVED_VALUE;

    int statEval;
    PackedMove bestMove{};
    nodeType nType = upperBound;
    ++_visitedNodes;

    // reading Transposition table for the best move
    auto& prevSearchRes = TTable.GetRecord(zHash);

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

            if (wasTTHit)
                prevSearchRes.SetStatVal(statEval);

            return bestEval;
        }

        alpha = bestEval;
    }

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

    // saving volatile fields
    VolatileBoardData oldData{bd};

    // Empty move cannot be a capture move se we are sure that valid move is saved
    if (wasTTHit && prevSearchRes.GetMove().IsCapture())
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i != 0)
            _fetchBestMove(moves, i);

        zHash = ProcessAttackMove(bd, moves[i], zHash, oldData, _repMap);
        const int moveValue = -_quiescenceSearch(bd, -beta, -alpha, zHash);
        zHash = RevertMove(bd, moves[i], zHash, oldData, _repMap);

        // if there was call to abort then abort
        if (std::abs(moveValue) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

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

    if (prevSearchRes.GetDepth() == 0 || (!wasTTHit && _age - prevSearchRes.GetAge() >= QUIESENCE_AGE_DIFF_REPLACE))
    {
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statEval, 0, nType, _age};
        TTable.Add(record, zHash);
    }
    return bestEval;
}

int BestMoveSearch::_zwQuiescenceSearch(Board &bd, const int alpha, uint64_t zHash)
{
    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;


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
            statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);
    }
    else
        statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);

    int bestEval = statEval;
    if (bestEval >= beta)
    {
        ++_cutoffNodes;

        if (wasTTHit)
            prevSearchRes.SetStatVal(statEval);

        return bestEval;
    }

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

    // saving volatile fields
    VolatileBoardData oldData{bd};

    // Empty move cannot be a capture move se we are sure that valid move is saved
    if (wasTTHit && prevSearchRes.GetMove().IsCapture())
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i != 0)
            _fetchBestMove(moves, i);

        zHash = ProcessAttackMove(bd, moves[i], zHash, oldData, _repMap);
        const int moveValue = -_zwQuiescenceSearch(bd, -beta, zHash);
        zHash = RevertMove(bd, moves[i], zHash, oldData, _repMap);

        // if there was call to abort then abort
        if (std::abs(moveValue) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

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

    if (prevSearchRes.GetDepth() == 0 || (!wasTTHit && _age - prevSearchRes.GetAge() >= QUIESENCE_AGE_DIFF_REPLACE))
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
    int maxValue  = NEGATIVE_INFINITY;
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
    return NEGATIVE_INFINITY + distToRoot;
}
