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
ProcessMove(Board &bd, const Move mv, const uint64_t hash, const VolatileBoardData &data)
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
            _histTable.ScaleTableDown();
            _maxPlyReached = 0;

            // performs the search without aspiration window to gather some initial statistics about the move
            eval = _search<SearchType::PVSearch, true>(
                NEGATIVE_INFINITY - 1, POSITIVE_INFINITY + 1, depth * FULL_DEPTH_FACTOR, 0, zHash, {}, pvBuff, nullptr
            );
            TraceIfFalse(_pv.IsFilled(), "PV buffer is not filled after the search!");

            // if there was call to abort then abort
            if (std::abs(eval) == TIME_STOP_RESERVED_VALUE)
                break;

            // saving the move evaluation to the avg value
            // TODO: maybe check previous pv for the predicting value?
            avg += depth * eval;
            _pv.Clone(pvBuff);
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
                "info depth {} seldepth {}/{} time {} nodes {} nps {} score cp {} currmove {} hashfull {} cut-offs perc "
                "{:.2f} pv ",
                depth, _maxPlyReached, _maxPlyReachedWithQSearch, spentMs, _visitedNodes, nps, IsMateScore(eval) ? eval : eval * SCORE_GRAIN,
                _pv[0].GetLongAlgebraicNotation(), TTable.GetContainedElements(), cutOffPerc
            );

            _pv.Print(eval == 0);
            GlobalLogger.LogStream << std::endl;
        }

        // Stop search if we already found a mate
        if (IsMateScore(eval))
            break;
    }

    TraceIfFalse(_stack.Size() == 0, "Stack is not empty after the search!");

    if constexpr (TestTT)
        TTable.DisplayStatisticsAndReset();

    if constexpr (CollectSearchData)
        _collectedData.DisplayData();

    return prevEval;
}

template <BestMoveSearch::SearchType searchType, bool followPv>
int BestMoveSearch::_search(
    int alpha, const int beta, int depthLeft, const int ply, uint64_t zHash, const Move prevMove, PV &pv, PackedMove *bestMoveOut
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

    // when we exhausted this branch start static evaluation with qsearch, additionally
    // when we exceeded maximally allowed depth simply return without consideration
    const int plyDepth = depthLeft / FULL_DEPTH_FACTOR;
    if (plyDepth <= 0 || ply >= MAX_SEARCH_DEPTH)
        return _qSearch<searchType>(alpha, beta, ply, zHash, 0, prevMove);

    // incrementing nodes counter;
    ++_visitedNodes;

    // Check whether we reached end of the legal path
    // Note:
    //   ply != 0 : prohibit behavior to return null move in case of draw by repetitions
    if (ply != 0 && _moveGenerator.IsDrawByReps(zHash))
        return DRAW_SCORE;

    // reading Transposition table for the best move
    auto& prevSearchRes = TTable.GetRecord(zHash);

    // check whether hashes are same
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);

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

    int statEval{};
    const bool isCheck = _board.IsCheck || _moveGenerator.IsCheck();

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
                const int qValue = _qSearch<SearchType::NoPVSearch>(alpha, beta, ply, zHash, 0, prevMove);
                if (qValue < beta)
                    return qValue;
            }
        }

    // generate moves
    auto moves = _moveGenerator.GetPseudoLegalMoves(_cmTable.GetCounterMove(prevMove, _board.MovingColor), ply, prevMove.GetTargetField());

    // saving volatile board state
    const VolatileBoardData oldData{_board};

    // Note: moves deduced from fail-low nodes are not good enough quality to use them in search
    // Note: when depth is equal to 0 there is an empty entry somehow or the stored moves was saved in qsearch,
    //       what means that no all moves where checked during the search and we should discard the move
    const bool isTTMoveUsable = wasTTHit && prevSearchRes.GetNodeType() != UPPER_BOUND && prevSearchRes.GetDepth() != 0;

    // Stores best move which at least exceeded initial alpha
    Move bestMove{};

    // Stores best evaluation found in this node
    int bestEval = NEGATIVE_INFINITY;

    // Used only for PV nodes
    [[maybe_unused]] PV inPV{};

    // clear killer table for the childs
    _kTable.ClearPlyFloor(ply);

    // NOTE: legal move is needed for draw if test at the end.
    // NOTE: moveCount is mainly used with multi-cut
    int moveCount{}; // counts legal moves that were searched
    int legalMoves{}; // Counts only legal moves

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
                    || (wasTTHit && !isTTMoveUsable))
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
                else if (isTTMoveUsable)
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

        // skip illegal moves
        if (!MoveGenerator::IsLegal(_board, moves[i]))
            continue;
        legalMoves++;

        // if the fetch move was excluded singular search simple skip this iteration
        if (moves[i].GetPackedMove() == _excludedMove)
            continue;

        int extensions{};
        int SEEValue = NEGATIVE_INFINITY;

        // ---------------------------- pruning -----------------------------------
        // we should avoid pruning when returning a mate score is possible
        // Note: ply > 0 - ensures no move is cut on root node to assure all  subtrees were checked
        // Note: moveCount != 0 - ensures that at least one move was tested before returning if there was legal one
        // Note: !IsMateScore(alpha) - ensures we did not accidentally cut a checkmate
        if (ply > 0 && moveCount != 0 && !IsMateScore(alpha))
        {
            // consider pruning of checks and captures with bad enough SEE value
            if (moves[i].IsAttackingMove() || moves[i].IsChecking())
            {
                SEEValue = _moveGenerator.SEE(moves[i]);

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
            // Note: ply > 0 - ensures we do not accidentally cut a move at root
            // Note: wasTTHit - ensures we have strong move saved from previous search
            // Note: _excludedMove.IsEmpty() - ensures no nested singular search is allowed
            // Note: moves[i].GetPackedMove() == prevSearchRes.GetMove() - ensures that only the strong move is searched
            // Note: plyDepth - prevSearchRes.GetDepth() <= SINGULAR_EXTENSION_DEPTH_PROBE_LIMIT - ensures
            //       minimal quality of move probed from the TT
            // Note: (prevSearchRes.GetNodeType() == LOWER_BOUND || prevSearchRes.GetNodeType() == PV_NODE) - ensures
            //       we skip all-nodes in singular search
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
                const int singularValue = _search<SearchType::NoPVSearch, false>(
                    singularBeta - 1, singularBeta, singularDepth, ply, zHash, prevMove, _dummyPv, nullptr
                );
                _excludedMove = PackedMove{};

                // we failed low, so our hypothesis might be true
                if (singularValue < singularBeta)
                    extensions += SINGULAR_EXTENSION;
                // Multi-cut pruning
                else if (singularBeta >= beta)
                    return (_stack.PopAggregate(moves), singularBeta);
            }

            // simple extensions deduction
            extensions += _deduceExtensions(prevMove, moves[i], SEEValue, IsPvNode);
        }

        // apply the moves changes to the board:
        zHash        = ProcessMove(_board, moves[i], zHash, oldData);
        // Note: increment counter of checked  childs in this node
        ++moveCount;

        //
        int reductions{};
        // -------------------------- reductions --------------------------------
        // determine whether we should spend less time in this node

        // Simplified LMR initial reduction
        reductions = CalcReductions(plyDepth, moveCount, beta - alpha);

        // Decrease reductions for nodes which was on previous PV to increase searched nodes on good paths
        if constexpr (followPv)
            reductions -= FULL_DEPTH_FACTOR;

        // Killer moves that refuted neigbhoring nodes has high probability to refute in this ndoe
        if (_kTable.IsKillerMove(moves[i], ply))
            reductions -= FULL_DEPTH_FACTOR;

        // Do not extend tactical moves that has high probability to be bad quality moves
        if (moves[i].IsAttackingMove() || moves[i].GetPackedMove().IsPromo() || moves[i].IsChecking())
        {
            if (SEEValue == NEGATIVE_INFINITY) SEEValue = _moveGenerator.SEE(moves[i]);

            // increase reduction only in case of positive SEEValue
            if (SEEValue > 0)
                reductions -= FULL_DEPTH_FACTOR;
        }

        // stores the most recent return value of child trees,
        int moveEval = alpha;

        // Late Move reductions
        // moveCount > 2 - we want to be sure to explore move from TT and best move accordingly to sorting
        if (!DisableLmr && moveCount > 2 && depthLeft >= LMR_MIN_DEPTH && reductions > 0)
        {
            const int LMRDepth = depthLeft + extensions - reductions - FULL_DEPTH_FACTOR;

            moveEval = -_search<SearchType::NoPVSearch, false>(
                    -(alpha + 1), -alpha, LMRDepth, ply + 1, zHash, moves[i], _dummyPv,
                    nullptr
            );

            // Perform research when move failed high
            if (moveEval > alpha)
            {
                moveEval = -_search<SearchType::NoPVSearch, false>(
                        -(alpha + 1), -alpha, depthLeft - FULL_DEPTH_FACTOR + extensions, ply + 1, zHash, moves[i], _dummyPv,
                        nullptr
                );
            }
        }
        // In pv nodes we always search first move on full window due to assumption that TT will give
        // us best move that is possible.
        // In case of non pv nodes we only search with zw
        else if (!IsPvNode || moveCount != 1)
        {
            moveEval = -_search<SearchType::NoPVSearch, false>(
                -(alpha + 1), -alpha, depthLeft - FULL_DEPTH_FACTOR + extensions, ply + 1, zHash, moves[i], _dummyPv,
                nullptr
            );
        }

        // if not, research move only in case of pv nodes
        if (IsPvNode && (moveCount == 1 || alpha < moveEval))
        {
            // Research with full window
            if (followPv && moveCount == 1)
                moveEval = -_search<SearchType::PVSearch, true>(
                    -beta, -alpha, depthLeft - FULL_DEPTH_FACTOR, ply + 1, zHash, moves[i], inPV, nullptr
                );
            else
                moveEval = -_search<SearchType::PVSearch, false>(
                    -beta, -alpha, depthLeft - FULL_DEPTH_FACTOR, ply + 1, zHash, moves[i], inPV, nullptr
                );
        }

        // move reverted after possible research
        zHash = RevertMove(_board, moves[i], zHash, oldData);

        // if there was call to abort then abort
        if (std::abs(moveEval) == TIME_STOP_RESERVED_VALUE)
            return (_stack.PopAggregate(moves), TIME_STOP_RESERVED_VALUE);

        // Check whether we should update values
        if (moveEval > bestEval)
        {
            bestEval = moveEval;
            if (moveEval > alpha)
            {
                bestMove = moves[i];

                // cut-off found
                if (moveEval >= beta)
                {
                    ++_cutoffNodes;

                    if constexpr (CollectSearchData)
                        _collectedData.SaveCutOff(legalMoves);

                    break;
                }

                // NOTE: in case of zero window search there is no possibility to improve alpha,
                //       therefore pv will not be updated
                alpha = bestEval;

                // Only inserts the move from the PV
                pv.InsertNext(bestMove.GetPackedMove(), inPV);
                // Additionaly clear the pv to prevent copying random moves in other iterations
                inPV.Clear();
            }
        }
    }

    // If no move is possible: check whether we hit mate or stalemate
    // TODO: consider how to act in case of excluded move and no legal moves due to lack of it
    if (legalMoves == 0)
        return (_stack.PopAggregate(moves), isCheck ? GetMateValue(ply) : DRAW_SCORE);

    // we found a good enough move update info about it
    if (bestEval > alpha && bestMove.IsQuietMove())
        _saveQuietMoveInfo(bestMove, prevMove, plyDepth, ply);

    // Note: compilation flag
    if constexpr (CollectSearchData)
        if (bestEval < beta) _collectedData.SaveNotCutOffNode(bestMove.IsEmpty() ? UPPER_BOUND : PV_NODE);

    // updating if profitable
    // NOTE: _excludedMove.IsEmpty() - ensures no move from singular search is saved
    if (_excludedMove.IsEmpty() && (plyDepth >= prevSearchRes.GetDepth() ||
                                    (!wasTTHit && _board.Age - prevSearchRes.GetAge() >= DEFAULT_AGE_DIFF_REPLACE)))
    {
        const NodeType nType = (bestEval >= beta ? LOWER_BOUND : bestMove.IsEmpty() ? UPPER_BOUND : PV_NODE);

        const TranspositionTable::HashRecord record{
            zHash,    bestMove.GetPackedMove(), bestEval,   statEval,
            plyDepth, nType,    _board.Age, ply
        };

        TTable.Add(record, zHash);
    }

    // Save move if possible
    if (bestMoveOut != nullptr)
        *bestMoveOut = bestMove.GetPackedMove();

    // clean up
    return (_stack.PopAggregate(moves), bestEval);
}

template <BestMoveSearch::SearchType searchType>
int BestMoveSearch::_qSearch(int alpha, const int beta, const int ply, uint64_t zHash, const int extendedDepth, const Move prevMove)
{
    static constexpr bool IsPvNode = searchType == SearchType::PVSearch;
    TraceIfFalse(beta > alpha, "Beta is not greater than alpha");
    if constexpr (!IsPvNode)
        TraceIfFalse(beta == alpha + 1, "Invalid alpha/beta in zw search");

    // save max depth reached with qsence
    _maxPlyReachedWithQSearch = std::max(_maxPlyReachedWithQSearch, ply);

    // Check whether we reached end of the legal path
    if (_moveGenerator.IsDrawByReps(zHash))
        return DRAW_SCORE;

    // Return if max depth exceeded, additionally add penalty for check positions
    if (ply >= MAX_SEARCH_DEPTH)
        return BoardEvaluator::DefaultFullEvalFunction(_board, _board.MovingColor) +
                (_moveGenerator.IsCheck() ? -100 / SCORE_GRAIN : 0);

    // if we need to stop the search signal it
    if (GameTimeManager::GetShouldStop())
        return TIME_STOP_RESERVED_VALUE;

    // incrementing nodes counter
    ++_visitedNodes;

    _kTable.ClearPlyFloor(ply);
    int bestEval = NEGATIVE_INFINITY;
    int statEval = NO_EVAL_RESERVED_VALUE;
    MoveGenerator::payload moves;

    // reading Transposition table for the best move
    auto &prevSearchRes = TTable.GetRecord(zHash);

    // We got a hit
    const bool wasTTHit = prevSearchRes.IsSameHash(zHash);

    // Avoid static evaluation when king is checked
    // When we have a check we cannot use static evaluation at all due to possible dangers that may happen
    // that means we should resolve most of the lines with checks
    const bool isCheck = _board.IsCheck || _moveGenerator.IsCheck();

    // Note: IsCapture() will return true only if the stored move is not empty and is capture so no matter check state
    //       we are in we will use the move
    // Note: Otherwise if we are in check state and we can only use good moves (moves theh not comes from fail-low nodes)
    //       Additionally we do not care about depth because move from any depth is better than qsearch depth
    const bool isTTMoveUsable = (prevSearchRes.GetNodeType() != UPPER_BOUND && isCheck) || prevSearchRes.GetMove().IsCapture();

    // Note: prevent tt based cut-offs in case of checks due to complications made by reductions and extensions
    //       we should focus to provide best quality tactical estimation insideq search
    if (!isCheck)
    {
        if (wasTTHit)
        {
            // Check for tt cut-off in case of zw quiesce search
            if constexpr (!IsPvNode)
            {
                // Return only matching scores

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

                // Save the static evaluation for later usage
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

        moves = _moveGenerator.GetPseudoLegalMoves<true>();
    }
    else
    // we are inside the check, our king is not safe we should resolve all moves
    {
        // When there is check we need to go through every possible move to get a better view about the position
        moves = _moveGenerator.GetPseudoLegalMoves(_cmTable.GetCounterMove(prevMove, _board.MovingColor), ply, prevMove.GetTargetField());
    }

    // saving volatile fields
    const VolatileBoardData oldData{_board};
    PackedMove bestMove{};

    if (wasTTHit && isTTMoveUsable)
        _pullMoveToFront(moves, prevSearchRes.GetMove());
    else
        _fetchBestMove(moves, 0);

    // iterating through moves
    int moveCount{}; // amount of legal moves
    for (size_t i = 0; i < moves.size; ++i)
    {
        // First move should be brouth to front if there was a tt hit
        if (i != 0)
            _fetchBestMove(moves, i);

        if (!MoveGenerator::IsLegal(_board, moves[i]))
            continue;
        ++moveCount;

        // pruning on the move
        if (!isCheck)
        {
            const int SEEValue = _moveGenerator.SEE(moves[i]);
            /*                  DELTA PRUNING                              */

            // Increase delta in case of promotion
            const int delta = statEval + DELTA_PRUNING_SAFETY_MARGIN + SEEValue +
                        (moves[i].GetPackedMove().IsPromo() ? DELTA_PRUNING_PROMO : 0);

            // There is high possibility that there is no chance to improve our position
            if (statEval + delta < alpha && !_board.IsEndGame())
                continue;

            /*                  SEE capture value estimation                */
            if (SEEValue < SEE_GOOD_MOVE_BOUNDARY)
                continue;
        }

        zHash               = ProcessMove(_board, moves[i], zHash, oldData);
        const int moveValue = -_qSearch<searchType>(-beta, -alpha, ply + 1, zHash, extendedDepth + 1, moves[i]);
        zHash               = RevertMove(_board, moves[i], zHash, oldData);

        // if there was call to abort then abort
        if (std::abs(moveValue) == TIME_STOP_RESERVED_VALUE)
            return (_stack.PopAggregate(moves), TIME_STOP_RESERVED_VALUE);

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

    // we are in check and no moves are possible
    if (isCheck && moveCount == 0)
        return (_stack.PopAggregate(moves), GetMateValue(ply + extendedDepth));

    // Note: !(bestEval > alpha && bestMove.IsEmpty()) excludes situation when we improve score with stat eval
    //       but no move exceeded alpha. In such situation node type check is invalid.
    // Note: !wasTTHit if there was a hit there is no chance to improve anything due to depth 0 save.
    // Note: (prevSearchRes.GetDepth() == 0) ensures we save only empty records and other qsearch saved records.
    // Note: QUIESENCE_AGE_DIFF_REPLACE significantly bigger than usual threshold.
    if (!isCheck && !wasTTHit && !(bestEval > alpha && bestMove.IsEmpty()) &&
        ((prevSearchRes.GetDepth() == 0) || _board.Age - prevSearchRes.GetAge() >= QUIESENCE_AGE_DIFF_REPLACE))
    {
        const NodeType nType = (bestEval >= beta ? LOWER_BOUND : bestMove.IsEmpty() ? UPPER_BOUND : PV_NODE);

        const TranspositionTable::HashRecord record{zHash, bestMove, bestEval,   statEval,
                                                    0,     nType,    _board.Age, ply + extendedDepth};
        TTable.Add(record, zHash);
    }

    return (_stack.PopAggregate(moves), bestEval);
}

void BestMoveSearch::_pullMoveToFront(const MoveGenerator::payload moves, const PackedMove mv)
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
        const auto bestMove = moves.data[ind];
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
    const auto bestMove              = moves.data[maxInd];
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
    const uint64_t hash = ZHasher.GenerateHash(_board);

    return _qSearch<SearchType::PVSearch>(NEGATIVE_INFINITY, POSITIVE_INFINITY, 0, hash, 0, {});
}

int BestMoveSearch::_deduceExtensions(const Move prevMove, const Move actMove, const int seeValue, const bool isPv)
    const
{
    int rv{};

    const ChessMechanics mech{_board};
    // check extensions

    rv += (actMove.IsChecking() && (seeValue == NEGATIVE_INFINITY ? mech.SEE(actMove) : seeValue) > 0) *
          (isPv ? CHECK_EXTENSION_PV_NODE : CHECK_EXTENSION);
    if constexpr (TraceExtensions)
        if (rv != 0) TraceWithInfo("Applied check extension");

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
