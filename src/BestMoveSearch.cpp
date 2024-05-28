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

using RepMap = std::unordered_map<uint64_t, int>;

#ifndef NDEBUG

#define TestNullMove() TraceIfFalse(!record._madeMove.IsEmpty(), "Received empty move inside the TT")
#define TestTTAdd()                                                                                                    \
    TraceIfFalse(record._type != NodeType::UPPER_BOUND, "Received upper board node inside the TT!");                   \
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

        if (UseAsp && depth < ASP_WND_MIN_DEPTH)
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
                {
                    if constexpr (TestAsp)
                        stat.RecordFinalBoundaries(alpha, beta, eval);

                    break;
                }
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

template <BestMoveSearch::SearchType searchType>
int BestMoveSearch::_search(
    Board &bd, int alpha, int beta, int depthLeft, uint64_t zHash, Move prevMove, PV &pv, bool followPv
)
{
    static constexpr bool IsPvNode = searchType == SearchType::PVSearch;
    TraceIfFalse(alpha < beta, "Alpha is not less than beta");
    if constexpr (!IsPvNode)
        TraceIfFalse(beta == alpha + 1, "Invalid alpha/beta in zw search");

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    // last depth static eval needed or prev pv node value
    if (depthLeft == 0)
        return _qSearch<searchType>(bd, alpha, beta, zHash, 0);

    // Check whether we reached end of the legal path
    if (_isDrawByReps(zHash))
        return DRAW_SCORE;

    // incrementing nodes counter;
    ++_visitedNodes;

    // reading Transposition table for the best move
    const auto prevSearchRes = TTable.GetRecord(zHash);

    // check whether hashes are same
    bool wasTTHit = prevSearchRes.IsSameHash(zHash);

    // When we missed the TT read we should try IID to save our situation
    if (!wasTTHit && !followPv && depthLeft >= IID_MIN_DEPTH)
    {
        _search<searchType>(bd, alpha, beta, depthLeft - IID_REDUCTION, zHash, prevMove, pv, false);

        // Retry TT read
        wasTTHit = prevSearchRes.IsSameHash(zHash);
    }

    if constexpr (TestTT)
        TTable.UpdateStatistics(wasTTHit);

    // Try to get a cut-off from tt record if the node is not pv node
    if constexpr (!IsPvNode)
        if (wasTTHit && prevSearchRes.GetDepth() >= depthLeft)
        {
            if (prevSearchRes.GetNodeType() == PV_NODE)
                return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(depthLeft, _currRootDepth);

            if (prevSearchRes.GetNodeType() == LOWER_BOUND && prevSearchRes.GetEval() >= beta)
                return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(depthLeft, _currRootDepth);

            if (prevSearchRes.GetNodeType() == UPPER_BOUND && prevSearchRes.GetEval() <= alpha)
                return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(depthLeft, _currRootDepth);
        }

    // generate moves
    MoveGenerator mechanics(
        bd, _stack, _histTable, _kTable, _cmTable.GetCounterMove(prevMove), depthLeft, prevMove.GetTargetField()
    );
    auto moves = mechanics.GetMovesFast();

    // If no move is possible: check whether we hit mate or stalemate
    if (moves.size == 0)
        return mechanics.IsCheck() ? GetMateValue(depthLeft, _currRootDepth) : DRAW_SCORE;

    // saving volatile board state
    VolatileBoardData oldData{bd};
    PackedMove bestMove{};
    int bestEval = NEGATIVE_INFINITY;
    PV inPV{depthLeft};

    // processing each move
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i == 0)
        {
            if (IsPvNode && followPv && depthLeft != 1)
                // first follow pv from previous ID (Iterative deepening) iteration,
                // we follow only single PV we previously saved
                _pullMoveToFront(moves, _pv(depthLeft, _currRootDepth));
            else if (wasTTHit && prevSearchRes.GetNodeType() != UPPER_BOUND && prevSearchRes.GetDepth() != 0)
                // if we have any move saved from last time we visited that node and the move is valid try to use it
                // NOTE: We don't store moves from fail low nodes only score so the move from fail low should always be
                // empty
                //       In other words we don't use best move from fail low nodes
                _pullMoveToFront(moves, prevSearchRes.GetMove());
            else
                _fetchBestMove(moves, i);
        }
        else
            _fetchBestMove(moves, i);

        // stores the most recent return value of child trees,
        // alpha + 1 value enforces the second if trigger in first iteration in case of pv nodes
        int moveEval = alpha + 1;
        zHash        = ProcessMove(bd, moves[i], depthLeft - 1, zHash, _kTable, oldData, _repMap);

        // In pv nodes we always search first move on full window due to assumption that TT will give
        // us best move that is possible.
        // In case of non pv nodes we only search with zw
        if (!IsPvNode || i != 0)
        {
            moveEval = -_search<SearchType::NoPVSearch>(
                bd, -(alpha + 1), -alpha, depthLeft - 1, zHash, moves[i], _dummyPv, false
            );
        }

        // if not, research move only in case of pv nodes
        if (IsPvNode && alpha < moveEval)
        {
            TTable.Prefetch(zHash);
            _kTable.ClearPlyFloor(depthLeft - 1);

            // Research with full window
            moveEval = -_search<SearchType::PVSearch>(
                bd, -beta, -alpha, depthLeft - 1, zHash, moves[i], inPV, followPv && i == 0
            );
        }

        // if there was call to abort then abort
        if (std::abs(moveEval) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

        // move reverted after possible research
        zHash = RevertMove(bd, moves[i], zHash, oldData, _repMap);

        // Check whether we should update values
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
                    break;
                }

                alpha = bestEval;
                pv.InsertNext(bestMove, inPV);
            }
        }
    }

    // updating if profitable
    if (depthLeft >= prevSearchRes.GetDepth() ||
        (!wasTTHit && bd.Age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE))
    {
        const NodeType nType = (bestEval >= beta ? LOWER_BOUND : bestMove.IsEmpty() ? UPPER_BOUND : PV_NODE);

        const TranspositionTable::HashRecord record{zHash,     bestMove,
                                                    bestEval,  wasTTHit ? prevSearchRes.GetStatVal() : NO_EVAL,
                                                    depthLeft, nType,
                                                    bd.Age,    _currRootDepth};

        TTable.Add(record, zHash);
    }

    // clean up
    _stack.PopAggregate(moves);
    return bestEval;
}

template <BestMoveSearch::SearchType searchType>
int BestMoveSearch::_qSearch(Board &bd, int alpha, int beta, uint64_t zHash, int extendedDepth)
{
    static constexpr bool IsPvNode = searchType == SearchType::PVSearch;
    TraceIfFalse(beta > alpha, "Beta is not greater than alpha");
    if constexpr (!IsPvNode)
        TraceIfFalse(beta == alpha + 1, "Invalid alpha/beta in zw search");

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    // Check whether we reached end of the legal path
    if (_isDrawByReps(zHash))
        return DRAW_SCORE;

    int bestEval = NEGATIVE_INFINITY;
    int statEval = NO_EVAL;
    MoveGenerator::payload moves;
    ++_visitedNodes;

    // reading Transposition table for the best move
    auto &prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);

    // When we have a check we cannot use static evaluation at all due to possible dangers that may happen
    // that means we should resolve most of the lines with checks
    MoveGenerator mechanics(bd, _stack);
    const bool isCheck = mechanics.IsCheck();

    // Avoid static evaluation when king is checked
    if (!isCheck)
    {
        if (wasTTHit)
        {
            // Check for tt cut-off in case of zw quiesce search
            if constexpr (!IsPvNode)
            {
                if (prevSearchRes.GetNodeType() == PV_NODE)
                    return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(-extendedDepth, _currRootDepth);

                if (prevSearchRes.GetNodeType() == LOWER_BOUND && prevSearchRes.GetEval() >= beta)
                    return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(-extendedDepth, _currRootDepth);

                if (prevSearchRes.GetNodeType() == UPPER_BOUND && prevSearchRes.GetEval() <= alpha)
                    return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(-extendedDepth, _currRootDepth);
            }

            // Try to read from tt previously calculated static evaluation
            if (prevSearchRes.GetStatVal() != NO_EVAL)
                statEval = prevSearchRes.GetStatVal();
            else
            {
                // otherwise calculate the static eval
                statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);
                TraceIfFalse(
                    statEval <= POSITIVE_INFINITY && statEval >= NEGATIVE_INFINITY,
                    "Received suspicious static evaluation points!"
                );

                prevSearchRes.SetStatVal(statEval);
            }
        }
        else
            // again no tt entry calculate eval
            statEval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);

        // check for stand-pat cut-off
        bestEval = statEval;
        if (bestEval > alpha)
        {
            if (bestEval >= beta)
                return ++_cutoffNodes, bestEval;

            alpha = bestEval;
        }

        moves = mechanics.GetMovesFast<true>();
    }
    else
    {
        // When there is check we need to go through every possible move to get a better view about the position
        moves = mechanics.GetMovesFast();

        if (moves.size == 0)
            return mechanics.IsCheck() ? GetMateValue(-extendedDepth, _currRootDepth) : DRAW_SCORE;
    }

    // saving volatile fields
    VolatileBoardData oldData{bd};
    PackedMove bestMove{};

    // Empty move cannot be a capture move se we are sure that valid move is saved
    if (wasTTHit && ((prevSearchRes.GetNodeType() != UPPER_BOUND && isCheck) || prevSearchRes.GetMove().IsCapture()))
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i != 0)
            _fetchBestMove(moves, i);

        zHash               = ProcessAttackMove(bd, moves[i], zHash, oldData, _repMap);
        const int moveValue = -_qSearch<searchType>(bd, -beta, -alpha, zHash, extendedDepth + 1);
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

    if (!isCheck && !wasTTHit &&
        (prevSearchRes.GetDepth() == 0 || bd.Age - prevSearchRes.GetAge() >= QUIESENCE_AGE_DIFF_REPLACE))
    {
        const NodeType nType = (bestEval >= beta ? LOWER_BOUND : bestMove.IsEmpty() ? UPPER_BOUND : PV_NODE);

        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval, statEval,
                                                    0,     nType,    bd.Age,   _currRootDepth};
        TTable.Add(record, zHash);
    }

    // clean up
    _stack.PopAggregate(moves);
    return bestEval;
}

int BestMoveSearch::_pwsSearch(
    Board &bd, int alpha, const int beta, const int depthLeft, uint64_t zHash, const Move prevMove, PV &pv,
    bool followPv
)
{
    return _search<SearchType::PVSearch>(bd, alpha, beta, depthLeft, zHash, prevMove, pv, followPv);
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
        auto bestMove = moves.data[ind];
        for (signed_size_t i = static_cast<signed_size_t>(ind) - 1; i >= 0; --i) moves.data[i + 1] = moves.data[i];
        moves.data[0] = bestMove;
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

    const auto signedTargetPos = static_cast<signed_size_t>(targetPos);
    auto bestMove              = moves.data[maxInd];
    for (signed_size_t i = static_cast<signed_size_t>(maxInd) - 1; i >= signedTargetPos; --i)
        moves.data[i + 1] = moves.data[i];
    moves.data[targetPos] = bestMove;

    TraceIfFalse(
        maxInd == targetPos || moves.data[targetPos].GetEval() >= moves.data[targetPos + 1].GetEval(),
        "Move sorting failed!"
    );
}

int BestMoveSearch::QuiesceEval()
{
    uint64_t hash = ZHasher.GenerateHash(_board);

    return _qSearch<SearchType::PVSearch>(_board, NEGATIVE_INFINITY, POSITIVE_INFINITY, hash, 0);
}