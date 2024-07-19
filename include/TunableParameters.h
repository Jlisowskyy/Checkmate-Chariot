//
// Created by Jlisowskyy on 7/19/24.
//

#ifndef CHECKMATE_CHARIOT_TUNABLEPARAMETERS_H
#define CHECKMATE_CHARIOT_TUNABLEPARAMETERS_H

#include "CompilationConstants.h"

#include <string>
#include <map>

// Stores list of functions changing global tunable parameters
struct GlobalParametersList : GlobalSingletonWrapper<GlobalParametersList>{
    ~GlobalParametersList() = default;

    void SetParameter(const std::string& param, const std::string& value)
    {
        if (_params.contains(param))
            _params[param](value);
    }

    void AddEntry(const std::string& param, void (*func)(const std::string&))
    {
        if (!_params.contains(param))
            _params[param] = func;
    }

    static void Init()
    {
        InitInstance(new GlobalParametersList());
    }

private:
    GlobalParametersList() = default;

    std::unordered_map<std::string, void (*)(const std::string&)> _params{};
};

template<typename ParamTypeT>
struct TunableParameter{


private:
    static ParamTypeT _param;
};

// ------------------------- EXTENSIONS ------------------------------
static constexpr int FULL_DEPTH_FACTOR                    = 4;
static constexpr int CHECK_EXTENSION_PV_NODE              = FULL_DEPTH_FACTOR;
static constexpr int CHECK_EXTENSION                      = FULL_DEPTH_FACTOR / 2;
static constexpr int PV_EXTENSION                         = FULL_DEPTH_FACTOR / 2;
static constexpr int EVEN_EXCHANGE_EXTENSION_PV_NODE      = FULL_DEPTH_FACTOR;
static constexpr int EVEN_EXCHANGE_EXTENSION              = FULL_DEPTH_FACTOR / 2;
static constexpr int SINGULAR_EXTENSION_DEPTH_PROBE_LIMIT = 3;
static constexpr int SINGULAR_EXTENSION_MIN_DEPTH         = 4;
static constexpr int SINGULAR_EXTENSION                   = 2 * FULL_DEPTH_FACTOR;
static constexpr int SINGULAR_EXTENSION_BETA_COEF      = 16 / SCORE_GRAIN;
static constexpr int SINGULAR_EXTENSION_BETA_DIV = 1;

// ------------------------------ REDUCTIONS --------------------------------------

static constexpr int LMR_MIN_DEPTH = 3 * FULL_DEPTH_FACTOR;

// limits maximal extensions inside the search tree to not overdo
constexpr bool ShouldExtend(const int ply, const int rootDepth) { return ply < 2 * rootDepth; }

constexpr int CalcReductions(const int depth, const int moveCount, const int scoreDelta)
{
    constexpr double SCALE_COEF = 400;

    // Base reduction calculating reduction base for expected fail nodes
    const int reductionBase =
            int(SCALE_COEF * std::sqrt(double(depth - 1)) * std::sqrt(double(moveCount - 1))) /  1024;

    // Decrease reduction for PV-Nodes we are at full window search
    return (scoreDelta != 1 ? 2 * reductionBase / 3 : reductionBase) * FULL_DEPTH_FACTOR;
}

// value below which SEE capture is considered bad
static constexpr int SEE_GOOD_MOVE_BOUNDARY = -115 / SCORE_GRAIN;

// value defines how much we reduce the search when no TT move is available
static constexpr int NO_TT_MOVE_REDUCTION = 1 * FULL_DEPTH_FACTOR;

// value of phase below game is considering to be an end-game
static constexpr int END_GAME_PHASE = 64;

/* Depth from which Internal Iterative Deepening (IID) is used */
static constexpr int IID_MIN_DEPTH_PLY_DEPTH = 5;

/* Ply reduction for IID case*/
static constexpr int IID_REDUCTION = 3 * FULL_DEPTH_FACTOR;

/* Minimal depth from which Aspiration Windows are used*/
static constexpr int ASP_WND_MIN_DEPTH = 7;

// Initial Aspiration Window Delta its cp value is equal to INITIAL_ASP_WINDOW_DELTA * BoardEvaluator::ScoreGrain
// (probably 8) ~= 48
static constexpr int INITIAL_ASP_WINDOW_DELTA = 3;
static constexpr int MAX_ASP_WINDOW_RETRIES       = 4;

static constexpr int LMR_TT_ALL_NODE_REDUCTION_MAX_DEPTH_DIFF = 5;

// ------------------------- PRUNING ----------------------------------------------------

// average pawn value + some part of average pawn
static constexpr int DELTA_PRUNING_SAFETY_MARGIN = (115 + 115) / SCORE_GRAIN;
// average queen value - average pawn value
static constexpr int DELTA_PRUNING_PROMO = (1000 - 115) / SCORE_GRAIN;

static constexpr int SEE_BASED_PRUNING_GOOD_MOVE_COEF = 8;

static constexpr bool ENABLE_RAZORING = true;
static constexpr int RAZORING_DEPTH = 3;
// queen killed + promoted + rook killed + safety margin
static constexpr int RAZORING_MARGIN = (900 + (900 - 100) + 500 + 100) / SCORE_GRAIN;

// ---------------------------------------------------------------------------------------


// -------------------------- HISTORY TABLE -----------------------------------------------

// Defines maximal points stored inside the history table during the search
static constexpr int HISTORY_TABLE_POINTS_LIMIT = 16*1024;

static constexpr int  HISTORY_TABLE_SCORE_COEF = 16;
static constexpr int  HISTORY_TABLE_SCORE_DIV = 1024;

// Bonus linear function parameters
static constexpr int HISTORY_BONUS_COEF = 2;
static constexpr int HISTORY_BONUS_BIAS = 0;

// Penalty linear function parameters
static constexpr int HISTORY_PENALTY_COEF = 1;
static constexpr int HISTORY_PENALTY_BIAS = 1;

static constexpr int HISTORY_SCALE_DOWN_FACTOR = 4;

static constexpr int LMR_GOOD_HISTORY_REDUCTION_DIV = (HISTORY_TABLE_POINTS_LIMIT / 2);

// -----------------------------------------------------------------------------------------

// --------------------------- Counter Move Table ------------------------------------------

static constexpr int COUNTER_MOVE_TABLE_PRIZE = 600;

// -----------------------------------------------------------------------------------------

// ---------------------------- Cont history tables ----------------------------------------

static constexpr int CONT_HISTORY_SCORE_TABLES_WRITE_COUNT = 4;
static constexpr int CONT_HISTORY_SCORE_TABLES_READ_COUNT = 3;

// -----------------------------------------------------------------------------------------

// ---------------------------- Direct Move Sort -------------------------------------------

static constexpr int MOVE_SORT_PROMO_CHECK = 100;

static constexpr int MOVE_SORT_CAPTURE_COEF = 1;
static constexpr int MOVE_SORT_CAPTURE_BIAS = 0;
static constexpr int MOVE_SORT_CAPTURE_DIV = 8;

static constexpr int MOVE_SORT_QUIET_CHECK = HISTORY_TABLE_POINTS_LIMIT / 8;

static constexpr int MOVE_SORT_GOOD_QUIET_DIV = 8;
static constexpr int MOVE_SORT_GOOD_QUIET_SCORE =
        // (HISTORY_TABLE_POINTS_LIMIT + COUNTER_MOVE_TABLE_PRIZE + MOVE_SORT_QUIET_CHECK) / MOVE_SORT_GOOD_QUIET_DIV;
        50;

static constexpr int MOVE_SORT_QUIETS_PAWN_EVASION_BONUS = 50;
static constexpr int MOVE_SORT_QUIETS_PAWN_DANGER_PENALTY = -50;

// -----------------------------------------------------------------------------------------

// ---------------------------- SINGULAR EXTENSIONS ----------------------------------------

// -----------------------------------------------------------------------------------------

// ---------------------------- Transpo Table params ---------------------------------------

static constexpr uint16_t QUIESENCE_AGE_DIFF_REPLACE = 16;
static constexpr uint16_t DEFAULT_AGE_DIFF_REPLACE   = 10;

// -----------------------------------------------------------------------------------------

#endif //CHECKMATE_CHARIOT_TUNABLEPARAMETERS_H
