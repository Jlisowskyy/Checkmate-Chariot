//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef COMPILATION_CONSTANTS_H
#define COMPILATION_CONSTANTS_H

#include <cinttypes>
#include <numeric>
#include <cmath>

using lli                  = long long int;
static constexpr size_t MB = 1024 * 1024;

// global defined Stack capacity used to store generated moves per thread
static constexpr size_t DEFAULT_STACK_SIZE = 32 * MB / sizeof(uint64_t);

/* Defines maximal depth of search allowed across the project */
static constexpr int MAX_SEARCH_DEPTH = 128;

/* Defines granularity of score returned by the static evaluation function */
static constexpr int SCORE_GRAIN = 4;

static constexpr uint64_t MSEC_TO_NSEC = 1000 * 1000;

static constexpr int16_t DRAW_SCORE         = 0;
static constexpr int16_t SPECIAL_DRAW_SCORE = 0;

static constexpr size_t MAX_MOVES = 256;
static constexpr size_t MAX_QUIET_MOVES = 128;

// ------------------------- EXTENSIONS ------------------------------
static constexpr int FULL_DEPTH_FACTOR                    = 4;
static constexpr int ONE_REPLY_EXTENSION_PV_NODE          = FULL_DEPTH_FACTOR;
static constexpr int ONE_REPLY_EXTENSION                  = FULL_DEPTH_FACTOR / 2;
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

// -------------------- RESERVED VALUES -------------------------------
static constexpr int RESERVED_SCORE_VALUES           = 64;
static constexpr int TIME_STOP_RESERVED_VALUE        = std::numeric_limits<int16_t>::max() - 1;
static constexpr int NO_EVAL_RESERVED_VALUE          = std::numeric_limits<int16_t>::max() - 2;
static constexpr int EVAL_DRAW_RESERVED_VALUE        = std::numeric_limits<int16_t>::max() - 3;
static constexpr int NEGATIVE_INFINITY               = std::numeric_limits<int16_t>::min() + RESERVED_SCORE_VALUES + 1;
static constexpr int POSITIVE_INFINITY               = std::numeric_limits<int16_t>::max() - RESERVED_SCORE_VALUES;
static constexpr int BEST_MATE_VALUE                 = NEGATIVE_INFINITY + MAX_SEARCH_DEPTH;
static constexpr int BEST_MATE_VALUE_ABS             = -(BEST_MATE_VALUE);
static constexpr uint16_t QUIESENCE_AGE_DIFF_REPLACE = 16;
static constexpr uint16_t DEFAULT_AGE_DIFF_REPLACE   = 10;

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
static constexpr int16_t INITIAL_ASP_WINDOW_DELTA = 3;
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
static constexpr int16_t HISTORY_TABLE_POINTS_LIMIT = 16*1024;

static constexpr int16_t  HISTORY_TABLE_SCORE_COEF = 16;
static constexpr int16_t  HISTORY_TABLE_SCORE_DIV = 1024;

// Bonus linear function parameters
static constexpr int16_t HISTORY_BONUS_COEF = 2;
static constexpr int16_t HISTORY_BONUS_BIAS = 0;

// Penalty linear function parameters
static constexpr int16_t HISTORY_PENALTY_COEF = 1;
static constexpr int16_t HISTORY_PENALTY_BIAS = 1;

static constexpr int16_t HISTORY_SCALE_DOWN_FACTOR = 4;

static constexpr int16_t LMR_GOOD_HISTORY_REDUCTION_DIV = (HISTORY_TABLE_POINTS_LIMIT / 2);

// -----------------------------------------------------------------------------------------

// --------------------------- Counter Move Table ------------------------------------------

static constexpr int16_t COUNTER_MOVE_TABLE_PRIZE = 600;

// -----------------------------------------------------------------------------------------

// ---------------------------- Cont history tables ----------------------------------------

static constexpr int16_t CONT_HISTORY_SCORE_TABLES_WRITE_COUNT = 4;
static constexpr int16_t CONT_HISTORY_SCORE_TABLES_READ_COUNT = 3;

// -----------------------------------------------------------------------------------------

// ---------------------------- Direct Move Sort -------------------------------------------

static constexpr int16_t MOVE_SORT_PROMO_CHECK = 100;

static constexpr int16_t MOVE_SORT_CAPTURE_COEF = 1;
static constexpr int16_t MOVE_SORT_CAPTURE_BIAS = 0;
static constexpr int16_t MOVE_SORT_CAPTURE_DIV = 8;

static constexpr int16_t MOVE_SORT_QUIET_CHECK = HISTORY_TABLE_POINTS_LIMIT / 8;

static constexpr int16_t MOVE_SORT_GOOD_QUIET_DIV = 8;
static constexpr int16_t MOVE_SORT_GOOD_QUIET_SCORE =
    // (HISTORY_TABLE_POINTS_LIMIT + COUNTER_MOVE_TABLE_PRIZE + MOVE_SORT_QUIET_CHECK) / MOVE_SORT_GOOD_QUIET_DIV;
    50;

static constexpr int16_t MOVE_SORT_QUIETS_PAWN_EVASION_BONUS = 50;
static constexpr int16_t MOVE_SORT_QUIETS_PAWN_DANGER_PENALTY = -50;

// -----------------------------------------------------------------------------------------

// ---------------------------- SINGULAR EXTENSIONS ----------------------------------------

// -----------------------------------------------------------------------------------------

/*
 * Three types of nodes that we can find during the search.
 * To get more information about them, please visit:
 * https://www.chessprogramming.org/Node_Types
 * */

enum NodeType : uint8_t
{
    PV_NODE,
    LOWER_BOUND,
    UPPER_BOUND
};

// ------------------------------
// Platform specific defines for force inline attribute

#ifndef PREVENT_FORCE_INLINE

#ifdef _MSC_VER

// MSVC specific force inline attribute
#define INLINE __forceinline

#else

// GNU / CLANG force inline specific attribute
#define INLINE __attribute__((always_inline))

#endif

#else

// No forced inline allowed
#define INLINE

#endif // PREVENT_FORCE_INLINE

// ------------------------------

// ------------------------------
// Platform specific defines for path separator

#ifdef __WIN32__

#define SLASH '\\'

#else

#define SLASH '/'

#endif

// ------------------------------

// ------------------------------
// Debug flag

#ifdef NDEBUG

static constexpr bool Debug = false;

#else

static constexpr bool Debug = true;

#endif // NDEBUG

//---------------------------

// ------------------------------
// Gather statistics about Transposition Table flow -
// enables code that gathers and displays information about every search run

#ifdef TEST_TT

static constexpr bool TestTT = true;

#else

static constexpr bool TestTT = false;

#endif // TEST_TT

//---------------------------

// ------------------------------
// Controls whether aspiration windows are used inside the search

#ifdef USE_ASP_WIN

static constexpr bool UseAsp = true;

#else

static constexpr bool UseAsp = false;

#endif // TEST_ASP_WIN

//---------------------------

// --------------------------
// Treat promo as single pawn -> queen move

#ifdef TREAT_PROMOS_AS_QUEEN_PROMO

static constexpr bool TreatPromosAsSingle = true;

#else

static constexpr bool TreatPromosAsSingle = false;

#endif // TREAT_PROMOS_AS_QUEEN_PROMO

// --------------------------

// ------------------------------
// Display statistics about aspiration window flow

#ifdef TEST_ASP_WIN

static constexpr bool TestAsp = UseAsp;

#else

static constexpr bool TestAsp = false;

#endif // TEST_ASP_WIN

//---------------------------

// --------------------------
// Trace extension changes

#ifdef TRACE_EXTENSIONS

static constexpr bool TraceExtensions = true;

#else

static constexpr bool TraceExtensions = false;

#endif // TRACE_EXTENSIONS
// --------------------------

// --------------------------
// Collect search data

#ifdef COLLECT_SEARCH_DATA

static constexpr bool CollectSearchData = true;

#else

static constexpr bool CollectSearchData = false;

#endif // COLLECT_SEARCH_DATA

// --------------------------

// --------------------------

#ifdef DISABLE_LMR

static constexpr bool DisableLmr = true;

#else

static constexpr bool DisableLmr = false;

#endif // DISABLE_LMR

// --------------------------

// --------------------------
// Display table data

#ifdef COLLECT_TABLE_DATA

static constexpr bool CollectTableData = true;

#else

static constexpr bool CollectTableData = false;

#endif // COLLECT_TABLE_DATA
// --------------------------

// ssize_t is defined by POSIX. Such define allows to use it on posix+windows
using signed_size_t = std::make_signed_t<size_t>;

#endif // COMPILATION_CONSTANTS_H
