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
#include "../include/Interface/FenTranslator.h"

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
ProcessAttackMove(Board &bd, const Move mv, const uint64_t hash, const VolatileBoardData &data)
{

    const uint64_t nextHash = ZHasher.UpdateHash(hash, mv, data);
    TTable.Prefetch(nextHash);
    Move::MakeMove(mv, bd);
    bd.Repetitions[nextHash]++;

    return nextHash;
}

[[nodiscard]] inline INLINE uint64_t ProcessMove(
    Board &bd, const Move mv, const int actualPly, const uint64_t hash, KillerTable &table,
    const VolatileBoardData &data
)
{

    const uint64_t nextHash = ZHasher.UpdateHash(hash, mv, data);
    TTable.Prefetch(nextHash);
    Move::MakeMove(mv, bd);
    bd.Repetitions[nextHash]++;

    return nextHash;
}

[[nodiscard]] inline INLINE uint64_t
RevertMove(Board &bd, const Move mv, const uint64_t hash, const VolatileBoardData &data)
{
    Move::UnmakeMove(mv, bd, data);

    if (const int occurs = --bd.Repetitions[hash]; occurs == 0)
        bd.Repetitions.erase(hash);

    return ZHasher.UpdateHash(hash, mv, data);
}

int BestMoveSearch::IterativeDeepening(
    PackedMove *bestMove, PackedMove *ponderMove, const int32_t maxDepth, const bool writeInfo
)
{
    // When the passed depth is 0, we need to evaluate the board statically
    if (maxDepth == 0)
    {
        const int score = BoardEvaluator::DefaultFullEvalFunction(_board, _board.MovingColor);
        GlobalLogger.LogStream << "info depth 0 score cp " << score << std::endl;

        return score;
    }

    // Generate unique hash for the board
    const uint64_t zHash = ZHasher.GenerateHash(_board);
    int32_t eval{};
    int32_t prevEval{};
    int64_t avg{};

    // prepare pv buffer
    PV pvBuff{};

    // usual search path
    const int range = std::min(maxDepth, MAX_SEARCH_DEPTH);
    for (int32_t depth = 1; depth <= range; ++depth)
    {
        // Search start time point
        [[maybe_unused]] auto timeStart = GameTimeManager::GetCurrentTime();

        // preparing variables used to display statistics
        _visitedNodes = 0;
        _cutoffNodes  = 0;
        _rootDepth    = depth;

        if (!UseAsp || depth < ASP_WND_MIN_DEPTH)
        {
            // cleaning tables used in previous iteration
            _kTable.ClearPlyFloor(0);
            _histTable.ScaleTableDown();
            _maxPlyReached = 0;

            // performs the search without aspiration window to gather some initial statistics about the move
            eval = _search<SearchType::PVSearch, true>(
                NEGATIVE_INFINITY - 1, POSITIVE_INFINITY + 1, depth * FULL_DEPTH_FACTOR, 0, zHash, {}, _pv, nullptr
            );
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
            [[maybe_unused]] AspWinStat stat{}; // Used only with TEST_ASP_WIN define

            // TODO: add some kind of logging inside the debug mode
            // Aspiration window loop
            while (true)
            {
                delta += delta;

                // cleaning tables used in previous iterations
                _kTable.ClearPlyFloor(0);
                _histTable.ScaleTableDown();
                _maxPlyReached = 0;
                eval           = _search<SearchType::PVSearch, true>(
                    alpha, beta, depth * FULL_DEPTH_FACTOR, 0, zHash, {}, pvBuff, nullptr
                );

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

        prevEval = eval;

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
                "info depth {} seldepth {} time {} nodes {} nps {} score cp {} currmove {} hashfull {} cut-offs perc "
                "{:.2f} pv ",
                depth, _maxPlyReached, spentMs, _visitedNodes, nps, IsMateScore(eval) ? eval : eval * SCORE_GRAIN,
                _pv[0].GetLongAlgebraicNotation(), TTable.GetContainedElements(), cutOffPerc
            );

            _pv.Print(eval == 0);
            GlobalLogger.LogStream << std::endl;
        }

        // Stop search if we already found a mate
        if (IsMateScore(eval))
            break;
    }

    // Ensure no memory leak when we received time stop and rolled back
    _stack.Clear();

    if constexpr (TestTT)
        TTable.DisplayStatisticsAndReset();

    return prevEval;
}

template <BestMoveSearch::SearchType searchType, bool followPv>
int BestMoveSearch::_search(
    int alpha, int beta, int depthLeft, int ply, uint64_t zHash, Move prevMove, PV &pv, PackedMove *bestMoveOut
)
{
    static constexpr bool IsPvNode = searchType == SearchType::PVSearch;
    TraceIfFalse(alpha < beta, "Alpha is not less than beta");
    if constexpr (!IsPvNode)
        TraceIfFalse(beta == alpha + 1, "Invalid alpha/beta in zw search");

    _maxPlyReached = std::max(_maxPlyReached, ply);

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    // last depth static eval needed or prev pv node value
    const int plyDepth = depthLeft / FULL_DEPTH_FACTOR;
    if (plyDepth <= 0)
        return _qSearch<searchType>(alpha, beta, ply, zHash, 0);

    // incrementing nodes counter;
    ++_visitedNodes;

    // Check whether we reached end of the legal path
    ChessMechanics mech{_board};
    if (mech.IsDrawByReps(zHash))
        return DRAW_SCORE;

    // reading Transposition table for the best move
    auto& prevSearchRes = TTable.GetRecord(zHash);

    // check whether hashes are same
    bool wasTTHit = prevSearchRes.IsSameHash(zHash);

    if constexpr (TestTT)
        TTable.UpdateStatistics(wasTTHit);

    // Try to get a cut-off from tt record if the node is not pv node.
    // The depth must be higher than in actual node to get high quality score
    // Additionally when we are in singular search we do not use cutoffs to prevent misinformation spread
    if constexpr (!IsPvNode)
        // Avoid TT cutoffs whenever we exclude some move, otherwise we might return score not true according to exclusion
        if (_excludedMove.IsEmpty() && wasTTHit && prevSearchRes.GetDepth() >= plyDepth)
        {
            if (prevSearchRes.GetNodeType() == PV_NODE)
                return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(ply);

            if (prevSearchRes.GetNodeType() == LOWER_BOUND && prevSearchRes.GetEval() >= beta)
                return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(ply);

            if (prevSearchRes.GetNodeType() == UPPER_BOUND && prevSearchRes.GetEval() <= alpha)
                return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(ply);
        }

    MoveGenerator mechanics(
            _board, _stack, _histTable, _kTable, _cmTable.GetCounterMove(prevMove), ply, prevMove.GetTargetField()
    );
    int statEval{};
    const bool isCheck = _board.IsCheck || mechanics.IsCheck();


    if (isCheck)
    // Do not evaluate statically position when in check
        statEval = NO_EVAL_RESERVED_VALUE;
    // Try to read static evaluation from the TT
    else if (wasTTHit)
    {
        if (prevSearchRes.GetStatVal() != NO_EVAL_RESERVED_VALUE)
            statEval = prevSearchRes.GetStatVal();
        else
        {
            statEval = BoardEvaluator::DefaultFullEvalFunction(_board, _board.MovingColor);

            // Additionally save the result to the table for later usage
            prevSearchRes.SetStatVal(statEval);
        }
    }
    else
        statEval = BoardEvaluator::DefaultFullEvalFunction(_board, _board.MovingColor);

    // ----------------------------------- RAZORING -------------------------------------------------
    if constexpr (!IsPvNode && ENABLE_RAZORING)
        if (!isCheck)
        {
            if (plyDepth <= RAZORING_DEPTH && statEval + RAZORING_MARGIN < beta)
            {
                const int qValue = _qSearch<SearchType::NoPVSearch>(alpha, beta, ply, zHash, 0);
                if (qValue < beta)
                    return qValue;
            }
        }

    // generate moves
    auto moves = mechanics.GetMovesFast();

    // If no move is possible: check whether we hit mate or stalemate
    if (moves.size == 0)
        return isCheck ? GetMateValue(ply) : DRAW_SCORE;

    // Extends paths where we have only one move possible
    // TODO: consider do it other way to detect it also on leafs
    if (ShouldExtend(ply, _rootDepth) && moves.size == 1)
    {
        depthLeft += IsPvNode ? ONE_REPLY_EXTENSION_PV_NODE : ONE_REPLY_EXTENSION;

        if constexpr (TraceExtensions)
            TraceWithInfo("Applied one reply extension");
    }

    // saving volatile board state
    VolatileBoardData oldData{_board};
    PackedMove bestMove{};
    int bestEval = NEGATIVE_INFINITY;
    PV inPV{};
    int moveCount{};

    // processing each move
    for (size_t i = 0; i < moves.size; ++i)
    {
        // determine move to check first
        if (i == 0)
        {
            // Only on pv nodes where pv following is enabled we fetch move from previously stored pv
            if (IsPvNode && followPv && _pv.Contains(ply))
            {
                // first follow pv from previous ID (Iterative deepening) iteration,
                // we follow only single PV we previously saved
                _pullMoveToFront(moves, _pv[ply]);

                // Extend pvs to detect changes earlier
                if (ShouldExtend(ply, _rootDepth) && ply % 2 == 1)
                {
                    depthLeft += PV_EXTENSION;

                    if (TraceExtensions)
                        TraceWithInfo("Applied pv extension");
                }
            }
            // otherwise fetch move the usual way
            else
            {
                PackedMove prevBestMove{};

                // IID (Internal Iteration Deepening ) in case of Transposition Table miss
                // or the found move is not good enough.
                // IMPORTANT NOTE:
                // We should avoid using IID when we exclude some move due to increased possibility of TT miss
                if (_excludedMove.IsEmpty()
                    && IsPvNode
                    && (!wasTTHit
                    || (wasTTHit && (prevSearchRes.GetNodeType() == UPPER_BOUND && prevSearchRes.GetDepth() == 0)))
                    && plyDepth >= IID_MIN_DEPTH_PLY_DEPTH)
                {
                    _search<searchType, false>(
                        alpha, beta, depthLeft - IID_REDUCTION, ply, zHash, prevMove, pv, &prevBestMove
                    );
                }
                // if we have any move saved from last time we visited that node and the move is valid try to use it
                // NOTE: We don't store moves from fail low nodes only score so the move from fail low should always
                // be empty
                //       In other words we don't use best move from fail low nodes
                else if (wasTTHit && prevSearchRes.GetNodeType() != UPPER_BOUND && prevSearchRes.GetDepth() != 0)
                    prevBestMove = prevSearchRes.GetMove();

                // try to pull found move to the front otherwise simply fetch move by heuristic eval
                if (!prevBestMove.IsEmpty())
                    _pullMoveToFront(moves, prevBestMove);
                else
                    _fetchBestMove(moves, i);
            }
        }
        // simply fetch move based on the heuristic eval
        else
            _fetchBestMove(moves, i);

        // if the fetch move was excluded singular search simple skip this iteration
        if (moves[i].GetPackedMove() == _excludedMove)
            continue;

        int extensions{};
        int SEEValue = NEGATIVE_INFINITY;

        // ---------------------------- pruning -----------------------------------
        // we should avoid pruning when returning a mate score is possible
        if (ply > 0 && i != 0 && !IsMateScore(alpha))
        {
            // consider pruning of checks and captures with bad enough SEE value
            if (moves[i].IsAttackingMove() || moves[i].IsChecking())
            {
                SEEValue = mechanics.SEE(moves[i]);

                if (SEEValue < (2 * SEE_GOOD_MOVE_BOUNDARY * plyDepth))
                    continue;
            }
        }

        // -------------------------- Extensions --------------------------------
        // determine whether we should spend more time in this node
        if (ShouldExtend(ply, _rootDepth))
        {
            // singular extensions:
            // we try to use entry from TT to determine whether given move is the only good move in this node,
            // if given thesis hold we extend proposed move search depth
            if (ply > 0 && wasTTHit && _excludedMove.IsEmpty() && moves[i].GetPackedMove() == prevSearchRes.GetMove() &&
                plyDepth - prevSearchRes.GetDepth() <= SINGULAR_EXTENSION_DEPTH_PROBE_LIMIT &&
                (prevSearchRes.GetNodeType() == LOWER_BOUND || prevSearchRes.GetNodeType() == PV_NODE) &&
                plyDepth > SINGULAR_EXTENSION_MIN_DEPTH)
            {
                // TODO: investigate deeply the depth
                const int singularDepth = (depthLeft - FULL_DEPTH_FACTOR) / 2;
                // TODO: might be mathematically optimised:
                // The singular beta is decreased by some margin to preserve high quality results of the search
                const int singularBeta = prevSearchRes.GetEval() - SINGULAR_EXTENSION_DEPTH_MARGIN * plyDepth;

                // NOTE: due to single excluded move no recursive singular searched are allowed
                _excludedMove           = prevSearchRes.GetMove();
                _kTable.ClearPlyFloor(ply + 1);
                const int singularValue = _search<SearchType::NoPVSearch, false>(
                    singularBeta - 1, singularBeta, singularDepth, ply, zHash, prevMove, _dummyPv, nullptr
                );
                _excludedMove = PackedMove{};

                // we failed low, so our hypothesis might be true
                if (singularValue < singularBeta)
                    extensions += SINGULAR_EXTENSION;
                // Multi-cut pruning
                else if (singularBeta >= beta)
                {
                    _stack.PopAggregate(moves);
                    return singularBeta;
                }
            }

            // simple extensions deduction
            extensions += _deduceExtensions(prevMove, moves[i], SEEValue, IsPvNode);
        }

        // apply the moves changes to the board:
        zHash        = ProcessMove(_board, moves[i], ply, zHash, _kTable, oldData);
        ++moveCount;

        int reductions{};
        // -------------------------- reductions --------------------------------
        // determine whether we should spend more time in this node

        // Simplified LMR
        reductions = CalcReductions(plyDepth, moveCount, beta - alpha);

        // Decrease reductions for nodes which was on previous PV
        if constexpr (followPv)
            reductions -= FULL_DEPTH_FACTOR;

        if constexpr (IsPvNode)
            reductions -= FULL_DEPTH_FACTOR;

        if (_kTable.IsKillerMove(moves[i], ply))
            reductions -= FULL_DEPTH_FACTOR;

        if (moves[i].IsAttackingMove() || moves[i].GetPackedMove().IsPromo())
        {
            if (SEEValue == NEGATIVE_INFINITY) SEEValue = mech.SEE(moves[i]);

            // increase reduction only in case of positive SEEValue
            if (SEEValue > 0)
                reductions -= FULL_DEPTH_FACTOR;
        }

        if (moves[i].IsChecking())
            reductions -= FULL_DEPTH_FACTOR;

        // stores the most recent return value of child trees,
        int moveEval = alpha;

        // Late Move reductions
        // i > 1 - we want to be sure to explore move from TT and best move accordingly to sorting
        if (reductions > 0 && depthLeft >= LMR_MIN_DEPTH && i > 1)
        {
            const int LMRDepth = depthLeft + extensions - reductions - FULL_DEPTH_FACTOR;

            _kTable.ClearPlyFloor(ply + 1);
            moveEval = -_search<SearchType::NoPVSearch, false>(
                    -(alpha + 1), -alpha, LMRDepth, ply + 1, zHash, moves[i], _dummyPv,
                    nullptr
            );

            // Perform research when move failed high
            if (moveEval > alpha)
            {
                _kTable.ClearPlyFloor(ply + 1);
                moveEval = -_search<SearchType::NoPVSearch, false>(
                        -(alpha + 1), -alpha, depthLeft - FULL_DEPTH_FACTOR + extensions, ply + 1, zHash, moves[i], _dummyPv,
                        nullptr
                );
            }
        }
        // In pv nodes we always search first move on full window due to assumption that TT will give
        // us best move that is possible.
        // In case of non pv nodes we only search with zw
        else if (!IsPvNode || i != 0)
        {
            _kTable.ClearPlyFloor(ply + 1);
            moveEval = -_search<SearchType::NoPVSearch, false>(
                -(alpha + 1), -alpha, depthLeft - FULL_DEPTH_FACTOR + extensions, ply + 1, zHash, moves[i], _dummyPv,
                nullptr
            );
        }

        // if not, research move only in case of pv nodes
        if (IsPvNode && (i == 0 || alpha < moveEval))
        {
            _kTable.ClearPlyFloor(ply + 1);

            // Research with full window
            if (followPv && i == 0)
                moveEval = -_search<SearchType::PVSearch, true>(
                    -beta, -alpha, depthLeft - FULL_DEPTH_FACTOR, ply + 1, zHash, moves[i], inPV, nullptr
                );
            else
                moveEval = -_search<SearchType::PVSearch, false>(
                    -beta, -alpha, depthLeft - FULL_DEPTH_FACTOR, ply + 1, zHash, moves[i], inPV, nullptr
                );
        }

        // if there was call to abort then abort
        if (std::abs(moveEval) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

        // move reverted after possible research
        zHash = RevertMove(_board, moves[i], zHash, oldData);

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
                        _saveQuietMoveInfo(moves[i], prevMove, plyDepth, ply);

                    ++_cutoffNodes;
                    break;
                }

                alpha = bestEval;
                pv.InsertNext(bestMove, inPV);
            }
        }
    }

    // updating if profitable
    if (_excludedMove.IsEmpty() && (plyDepth >= prevSearchRes.GetDepth() ||
                                    (!wasTTHit && _board.Age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE)))
    {
        const NodeType nType = (bestEval >= beta ? LOWER_BOUND : bestMove.IsEmpty() ? UPPER_BOUND : PV_NODE);

        const TranspositionTable::HashRecord record{
            zHash,    bestMove, bestEval,   statEval,
            plyDepth, nType,    _board.Age, ply
        };

        TTable.Add(record, zHash);
    }

    if (bestMoveOut != nullptr)
        *bestMoveOut = bestMove;

    // clean up
    _stack.PopAggregate(moves);
    return bestEval;
}

template <BestMoveSearch::SearchType searchType>
int BestMoveSearch::_qSearch(int alpha, int beta, int ply, uint64_t zHash, int extendedDepth)
{
    static constexpr bool IsPvNode = searchType == SearchType::PVSearch;
    TraceIfFalse(beta > alpha, "Beta is not greater than alpha");
    if constexpr (!IsPvNode)
        TraceIfFalse(beta == alpha + 1, "Invalid alpha/beta in zw search");

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    if (ply > _maxPlyReachedWithQSearch)
    {
        _maxPlyReachedWithQSearch = ply;

        if constexpr (Debug)
            TRACE_STACK_USAGE();
    }

    // incrementing nodes counter
    ++_visitedNodes;

    // Check whether we reached end of the legal path
    ChessMechanics mech{_board};
    if (mech.IsDrawByReps(zHash))
        return DRAW_SCORE;

    int bestEval = NEGATIVE_INFINITY;
    int statEval = NO_EVAL_RESERVED_VALUE;
    MoveGenerator::payload moves;

    // reading Transposition table for the best move
    auto &prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);

    // Avoid static evaluation when king is checked
    MoveGenerator mechanics(_board, _stack);

    // When we have a check we cannot use static evaluation at all due to possible dangers that may happen
    // that means we should resolve most of the lines with checks
    const bool isCheck = _board.IsCheck || mechanics.IsCheck();

    if (!isCheck)
    {
        if (wasTTHit)
        {
            // Check for tt cut-off in case of zw quiesce search
            if constexpr (!IsPvNode)
            {
                if (prevSearchRes.GetNodeType() == PV_NODE)
                    return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(ply + extendedDepth);

                if (prevSearchRes.GetNodeType() == LOWER_BOUND && prevSearchRes.GetEval() >= beta)
                    return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(ply + extendedDepth);

                if (prevSearchRes.GetNodeType() == UPPER_BOUND && prevSearchRes.GetEval() <= alpha)
                    return ++_cutoffNodes, prevSearchRes.GetAdjustedEval(ply + extendedDepth);
            }

            // Try to read from tt previously calculated static evaluation
            if (prevSearchRes.GetStatVal() != NO_EVAL_RESERVED_VALUE)
            {
                statEval = prevSearchRes.GetStatVal();
                BoardEvaluator::PopulateLastPhase(_board);
            }
            else
            {
                // otherwise calculate the static eval
                statEval = BoardEvaluator::DefaultFullEvalFunction(_board, _board.MovingColor);
                TraceIfFalse(
                    statEval <= POSITIVE_INFINITY && statEval >= NEGATIVE_INFINITY,
                    "Received suspicious static evaluation points!"
                );

                prevSearchRes.SetStatVal(statEval);
            }
        }
        else
            // again no tt entry calculate eval
            statEval = BoardEvaluator::DefaultFullEvalFunction(_board, _board.MovingColor);

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
    // we are inside the check, our king is not safe we should resolve all moves
    {
        // When there is check we need to go through every possible move to get a better view about the position
        moves = mechanics.GetMovesFast();

        // we are in check and no moves are possible
        if (moves.size == 0)
            return GetMateValue(ply + extendedDepth);
    }

    // saving volatile fields
    VolatileBoardData oldData{_board};
    PackedMove bestMove{};

    // avoid using moves from TT when inside singular search
    if (wasTTHit
       // Empty move cannot be a capture move se we are sure that valid move is saved
       && ((prevSearchRes.GetNodeType() != UPPER_BOUND && isCheck) || prevSearchRes.GetMove().IsCapture()))
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // iterating through moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        if (i != 0)
            _fetchBestMove(moves, i);

        // pruning on the move
        if (!isCheck)
        {
            const int SEEValue = mech.SEE(moves[i]);
            /*                  DELTA PRUNING                              */

            // Increase delta in case of promotion
            int delta = statEval + DELTA_PRUNING_SAFETY_MARGIN + SEEValue +
                        (moves[i].GetPackedMove().IsPromo() ? DELTA_PRUNING_PROMO : 0);

            // There is high possibility that there is no chance to improve our position
            if (statEval + delta < alpha && !_board.IsEndGame())
                continue;

            /*                  SEE capture value estimation                */
            if (SEEValue < SEE_GOOD_MOVE_BOUNDARY)
                continue;
        }

        zHash               = ProcessAttackMove(_board, moves[i], zHash, oldData);
        const int moveValue = -_qSearch<searchType>(-beta, -alpha, ply + 1, zHash, extendedDepth + 1);
        zHash               = RevertMove(_board, moves[i], zHash, oldData);

        // if there was call to abort then abort
        if (std::abs(moveValue) == TIME_STOP_RESERVED_VALUE)
            return TIME_STOP_RESERVED_VALUE;

        // Update node info
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

    // saving the move is only possible when no excluded move is used
    if (!isCheck && !wasTTHit &&
        ((prevSearchRes.GetDepth() == 0) || _board.Age - prevSearchRes.GetAge() >= QUIESENCE_AGE_DIFF_REPLACE))
    {
        const NodeType nType = (bestEval >= beta ? LOWER_BOUND : bestMove.IsEmpty() ? UPPER_BOUND : PV_NODE);

        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval,   statEval,
                                                    0,     nType,    _board.Age, ply + extendedDepth};
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

    return _qSearch<SearchType::PVSearch>(NEGATIVE_INFINITY, POSITIVE_INFINITY, 0, hash, 0);
}

int BestMoveSearch::_deduceExtensions(Move prevMove, Move actMove, const int seeValue, const bool isPv)
{
    int rv{};

    ChessMechanics mech{_board};
    // check extensions

    rv += (actMove.IsChecking() && (seeValue == NEGATIVE_INFINITY ? mech.SEE(actMove) : seeValue) > 0) *
          (isPv ? CHECK_EXTENSION_PV_NODE : CHECK_EXTENSION);
    if (TraceExtensions && rv != 0)
        TraceWithInfo("Applied check extension");

    // we are sure that actMove is attacking move because is moved to the same square as prevMove
    if (!prevMove.IsEmpty() && prevMove.GetTargetField() == actMove.GetTargetField() && prevMove.IsAttackingMove() &&
        BoardEvaluator::BasicFigureValues[prevMove.GetKilledBoardIndex()] +
                BoardEvaluator::BasicFigureValues[actMove.GetKilledBoardIndex()] ==
            0)
    {
        TraceIfFalse(actMove.IsAttackingMove(), "if check should not allow any non attacking moves to enter!!");

        rv += isPv ? EVEN_EXCHANGE_EXTENSION_PV_NODE : EVEN_EXCHANGE_EXTENSION;

        if constexpr (TraceExtensions)
            TraceWithInfo("Applied even exchange extension");
    }

    return rv;
}
