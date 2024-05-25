//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/Search/BestMoveSearch.h"

#include <chrono>
#include <format>
#include <unordered_map>
#include <vector>

#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/Search/TranspositionTable.h"
#include "../include/Search/ZobristHash.h"
#include "../include/TestsAndDebugging/DebugTools.h"
#include "../include/ThreadManagement/GameTimeManager.h"

static constexpr int NO_EVAL = TranspositionTable::HashRecord::NoEval;
using RepMap                 = std::unordered_map<uint64_t, int>;

#ifndef NDEBUG

#define TestNullMove() TraceIfFalse(!record._madeMove.IsEmpty(), "Received empty move inside the TT")
#define TestTTAdd()                                                                                                    \
    TraceIfFalse(record._type != NodeType::UPPER_BOUND, "Received upper board node inside the TT!");                    \
    TestNullMove()

#else
#define TestNullMove()
#define TestTTAdd()
#endif // NDEBUG

[[nodiscard]] inline INLINE uint64_t
ProcessAttackMove(Board &bd, const Move mv, const uint64_t hash, const VolatileBoardData &data, RepMap &rMap)
{

    const uint64_t nextHash = ZHasher.UpdateHash(hash, mv, data);
    TTable.Prefetch(nextHash);
    Move::MakeMove(mv, bd);
    rMap[nextHash]++;

    return nextHash;
}

[[nodiscard]] inline INLINE uint64_t ProcessMove(
    Board &bd, const Move mv, const int depth, const uint64_t hash, KillerTable &table, const VolatileBoardData &data,
    RepMap &rMap
)
{

    const uint64_t nextHash = ZHasher.UpdateHash(hash, mv, data);
    TTable.Prefetch(nextHash);
    Move::MakeMove(mv, bd);
    table.ClearPlyFloor(depth);
    rMap[nextHash]++;

    return nextHash;
}

[[nodiscard]] inline INLINE uint64_t
RevertMove(Board &bd, const Move mv, const uint64_t hash, const VolatileBoardData &data, RepMap &rMap)
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

        if (UseAsp && depth < 7)
        {
            // set according depth inside the pv buffer
            _pv.SetDepth(depth);

            // cleaning tables used in previous iteration
            _kTable.ClearPlyFloor(depth);
            _histTable.ScaleTableDown();

            // performs the search without aspiration window to gather some initial statistics about the move
            eval = _pwsSearch(_board, NEGATIVE_INFINITY - 1, POSITIVE_INFINITY + 1, depth, zHash, {}, _pv, true);
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

                    beta  = (alpha + beta) / 2;
                    alpha = std::max(eval - delta, NEGATIVE_INFINITY - 1);
                }
                else if (eval >= beta)
                {
                    if constexpr (TestAsp)
                        stat.RetryFailHigh(alpha, beta, eval);

                    // We failed high so move the upper boundary
                    beta = std::min(eval + delta, POSITIVE_INFINITY + 1);
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
                depth, spentMs, _visitedNodes, nps, IsMateScore(eval) ? eval : eval * BoardEvaluator::ScoreGrain,
                _pv[0].GetLongAlgebraicNotation(), TTable.GetContainedElements(), cutOffPerc
            );

            _pv.Print();
            GlobalLogger.LogStream << std::endl;
        }

        // Stop search if we already found a mate
        if (IsMateScore(eval))
            break;
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
    PackedMove bestMove{};

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _quiescenceSearch(bd, alpha, beta, zHash);

    // Check whether we reached end of the legal path
    if (_isDrawByReps(zHash))
        return DRAW_SCORE;

    // incrementing nodes counter;
    ++_visitedNodes;

    // generate moves
    MoveGenerator mechanics(
        bd, _stack, _histTable, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField()
    );
    auto moves = mechanics.GetMovesFast();

    // If no move is possible: check whether we hit mate or stalemate
    if (moves.size == 0)
        return mechanics.IsCheck() ? GetMateValue(depthLeft, _currRootDepth) : DRAW_SCORE;

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
    else if (wasTTHit && prevSearchRes.GetNodeType() != UPPER_BOUND && prevSearchRes.GetDepth() != 0)
        // if we have any move saved from last time we visited that node and the move is valid try to use it
        // NOTE: We don't store moves from fail low nodes only score so the move from fail low should always be empty
        //       In other words we don't use best move from fail low nodes
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // saving volatile board state
    VolatileBoardData oldData{bd};

    PV inPV{depthLeft};

    zHash        = ProcessMove(bd, moves[0], depthLeft - 1, zHash, _kTable, oldData, _repMap);
    int bestEval = -_pwsSearch(bd, -beta, -alpha, depthLeft - 1, zHash, moves[0], inPV, followPv);
    zHash        = RevertMove(bd, moves[0], zHash, oldData, _repMap);

    // if there was call to abort then abort
    if (std::abs(bestEval) == TIME_STOP_RESERVED_VALUE)
        return TIME_STOP_RESERVED_VALUE;

    if (bestEval > alpha)
    {
        // cut-off found
        if (bestEval >= beta)
        {
            ++_cutoffNodes;

            if (moves[0].IsQuietMove())
                _saveQuietMoveInfo(moves[0], prevMove, depthLeft);

            // updating if profitable
            if (depthLeft >= prevSearchRes.GetDepth() ||
                (!wasTTHit && bd.Age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE))
            {
                const TranspositionTable::HashRecord record{zHash, moves[0].GetPackedMove(),
                                                            bestEval,  wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL,
                                                            depthLeft, LOWER_BOUND,
                                                            bd.Age, _currRootDepth};
                TTable.Add(record, zHash);
                TestTTAdd();
            }

            _stack.PopAggregate(moves);
            return bestEval;
        }

        alpha    = bestEval;
        bestMove = moves[0].GetPackedMove();
        pv.InsertNext(bestMove, inPV);
    }

    // processing each move
    for (size_t i = 1; i < moves.size; ++i)
    {
        _fetchBestMove(moves, i);

        zHash        = ProcessMove(bd, moves[i], depthLeft - 1, zHash, _kTable, oldData, _repMap);
        int moveEval = -_zwSearch(bd, - (alpha + 1), depthLeft - 1, zHash, moves[i]);

        // if there was call to abort then abort
        if (std::abs(moveEval) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

        // if not, research move
        if (alpha < moveEval)
        {
            TTable.Prefetch(zHash);
            _kTable.ClearPlyFloor(depthLeft - 1);
            moveEval = -_pwsSearch(bd, -beta, -alpha, depthLeft - 1, zHash, moves[i], inPV, false);

            // if there was call to abort then abort
            if (std::abs(moveEval) == TIME_STOP_RESERVED_VALUE)
                return TIME_STOP_RESERVED_VALUE;

            // TODO: safety measures
            if (moveEval > bestEval)
            {
                bestEval = moveEval;

                if (moveEval > alpha)
                {
                    bestMove = moves[i].GetPackedMove();

                    // cut-off found
                    if (moveEval >= beta)
                    {
                        if (moves[i].IsQuietMove())
                            _saveQuietMoveInfo(moves[i], prevMove, depthLeft);

                        ++_cutoffNodes;
                        zHash = RevertMove(bd, moves[i], zHash, oldData, _repMap);
                        break;
                    }

                    alpha = bestEval;
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
    if (depthLeft >= prevSearchRes.GetDepth() ||
        (!wasTTHit && bd.Age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE))
    {
        const NodeType nType = (bestEval >= beta   ? LOWER_BOUND :
                                bestMove.IsEmpty() ? UPPER_BOUND :
                                                     PV_NODE);

        const TranspositionTable::HashRecord record{
            zHash, bestMove, bestEval, wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL, depthLeft, nType, bd.Age, _currRootDepth
        };

        TTable.Add(record, zHash);
    }

    return bestEval;
}

[[nodiscard]] int
BestMoveSearch::_zwSearch(Board &bd, const int alpha, const int depthLeft, uint64_t zHash, const Move prevMove)
{
    const int beta = alpha + 1;
    int bestEval   = NEGATIVE_INFINITY;
    PackedMove bestMove{};

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _zwQuiescenceSearch(bd, alpha, zHash, 0);

    // Check whether we reached end of the legal path
    if (_isDrawByReps(zHash))
        return DRAW_SCORE;

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
        if (prevSearchRes.GetNodeType() == PV_NODE)
            return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(depthLeft, _currRootDepth);

        if (prevSearchRes.GetNodeType() == LOWER_BOUND && prevSearchRes.GetEval() >= beta)
            return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(depthLeft, _currRootDepth);

        if (prevSearchRes.GetNodeType() == UPPER_BOUND && prevSearchRes.GetEval() <= alpha)
            return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(depthLeft, _currRootDepth);
    }

    // saving volatile fields
    VolatileBoardData oldData{bd};

    // generate moves
    MoveGenerator mechanics(
        bd, _stack, _histTable, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField()
    );
    auto moves = mechanics.GetMovesFast();

    if (moves.size == 0)
        return mechanics.IsCheck() ? GetMateValue(depthLeft, _currRootDepth) : DRAW_SCORE;

    // processing each move
    for (size_t i = 0; i < moves.size; ++i)
    {
        // load the best move from TT if possible, EmptyMove means UPPER_BOUND record which we want to avoid using
        // and additionally entries with depth 0 were saved during quiesce search so are not useful here
        if (i == 0 && wasTTHit && prevSearchRes.GetNodeType() != UPPER_BOUND && prevSearchRes.GetDepth() != 0)
            _pullMoveToFront(moves, prevSearchRes.GetMove());
        else
            _fetchBestMove(moves, i);

        zHash              = ProcessMove(bd, moves[i], depthLeft - 1, zHash, _kTable, oldData, _repMap);
        const int moveEval = -_zwSearch(bd, -beta, depthLeft - 1, zHash, moves[i]);
        zHash              = RevertMove(bd, moves[i], zHash, oldData, _repMap);

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

                bestMove = moves[i].GetPackedMove();
                ++_cutoffNodes;
                break;
            }
        }
    }

    if (depthLeft >= prevSearchRes.GetDepth() ||
        (!wasTTHit && bd.Age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE))
    {
        const NodeType nType = (bestEval >= beta ? LOWER_BOUND : UPPER_BOUND);

        const TranspositionTable::HashRecord record{
            zHash, bestMove, bestEval, wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL, depthLeft, nType, bd.Age, _currRootDepth
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

    // Check whether we reached end of the legal path
    if (_isDrawByReps(zHash))
        return DRAW_SCORE;

    int bestEval = NEGATIVE_INFINITY;
    PackedMove bestMove{};
    ++_visitedNodes;

    // reading Transposition table for the best move
    auto &prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);

//    ChessMechanics mech(bd);
//    const bool isCheck = mech.IsCheck();
    if (wasTTHit && prevSearchRes.GetStatVal() != NO_EVAL)
        bestEval = prevSearchRes.GetStatVal();
    else {
        bestEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);

        if (wasTTHit)
            prevSearchRes.SetStatVal(bestEval);
    }


    if (bestEval > alpha)
    {
        if (bestEval >= beta)
            return ++_cutoffNodes, bestEval;

        alpha = bestEval;
    }

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

    // saving volatile fields
    VolatileBoardData oldData{bd};

    // Empty move cannot be a capture move se we are sure that valid move is saved
    if (wasTTHit && prevSearchRes.GetNodeType() != UPPER_BOUND && prevSearchRes.GetMove().IsCapture())
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i != 0)
            _fetchBestMove(moves, i);

        zHash               = ProcessAttackMove(bd, moves[i], zHash, oldData, _repMap);
        const int moveValue = -_quiescenceSearch(bd, -beta, -alpha, zHash);
        zHash               = RevertMove(bd, moves[i], zHash, oldData, _repMap);

        // if there was call to abort then abort
        if (std::abs(moveValue) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

        if (moveValue > bestEval)
        {
            bestEval = moveValue;

            if (moveValue > alpha)
            {
                bestMove = moves[i].GetPackedMove();
                if (moveValue >= beta)
                {
                    ++_cutoffNodes;
                    break;
                }

                alpha = moveValue;
            }
        }
    }

    // clean up
    _stack.PopAggregate(moves);

    if (prevSearchRes.GetDepth() == 0 || (!wasTTHit && bd.Age - prevSearchRes.GetAge() >= QUIESENCE_AGE_DIFF_REPLACE))
    {
        const NodeType nType = (bestEval >= beta   ? LOWER_BOUND :
                                bestMove.IsEmpty() ? UPPER_BOUND :
                                PV_NODE);

        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statEval, 0, nType, bd.Age, _currRootDepth};
        TTable.Add(record, zHash);
    }

    return bestEval;
}

int BestMoveSearch::_zwQuiescenceSearch(Board &bd, const int alpha, uint64_t zHash, int extendedDepth)
{
    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    // Check whether we reached end of the legal path
    if (_isDrawByReps(zHash))
        return DRAW_SCORE;

    const int beta = alpha + 1;
    int statEval;
    PackedMove bestMove{};
    ++_visitedNodes;

    // reading Transposition table for the best move
    auto prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);
    if (wasTTHit)
    {
        if (prevSearchRes.GetNodeType() == PV_NODE)
            return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(-extendedDepth, _currRootDepth);

        if (prevSearchRes.GetNodeType() == LOWER_BOUND && prevSearchRes.GetEval() >= beta)
            return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(-extendedDepth, _currRootDepth);

        if (prevSearchRes.GetNodeType() == UPPER_BOUND && prevSearchRes.GetEval() <= alpha)
            return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(-extendedDepth, _currRootDepth);

        if (prevSearchRes.GetStatVal() != NO_EVAL)
            statEval = prevSearchRes.GetStatVal();
        else {
            statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);
            prevSearchRes.SetStatVal(statEval);
        }
    }
    else
        statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);

    int bestEval = statEval;
    if (bestEval >= beta)
        return ++_cutoffNodes, bestEval;

    // generating moves
    MoveGenerator mechanics(bd, _stack);
    auto moves = mechanics.GetMovesFast<true>();

    // saving volatile fields
    VolatileBoardData oldData{bd};

    // Empty move cannot be a capture move se we are sure that valid move is saved
    if (wasTTHit && prevSearchRes.GetNodeType() != UPPER_BOUND && prevSearchRes.GetMove().IsCapture())
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i != 0)
            _fetchBestMove(moves, i);

        zHash               = ProcessAttackMove(bd, moves[i], zHash, oldData, _repMap);
        const int moveValue = -_zwQuiescenceSearch(bd, -beta, zHash, extendedDepth + 1);
        zHash               = RevertMove(bd, moves[i], zHash, oldData, _repMap);

        // if there was call to abort then abort
        if (std::abs(moveValue) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

        if (moveValue > bestEval)
        {
            bestEval = moveValue;
            if (moveValue >= beta)
            {
                ++_cutoffNodes;

                bestMove = moves[i].GetPackedMove();
                break;
            }
        }
    }

    if (prevSearchRes.GetDepth() == 0 || (!wasTTHit && bd.Age - prevSearchRes.GetAge() >= QUIESENCE_AGE_DIFF_REPLACE))
    {
        const NodeType nType = (bestEval >= beta ? LOWER_BOUND : UPPER_BOUND);
        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statEval, 0, nType, bd.Age, _currRootDepth};
        TTable.Add(record, zHash);
    }

    // clean up
    _stack.PopAggregate(moves);
    return bestEval;
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

int BestMoveSearch::QuiesceEval() {
    uint64_t hash = ZHasher.GenerateHash(_board);

    return _quiescenceSearch(_board, NEGATIVE_INFINITY, POSITIVE_INFINITY, hash);
}
