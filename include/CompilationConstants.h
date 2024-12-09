//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef COMPILATION_CONSTANTS_H
#define COMPILATION_CONSTANTS_H

#include <cinttypes>
#include <numeric>
#include <cmath>
#include <cassert>
#include <cstddef>
#include <limits>

// Defines simple interface to communicate with objects with SINGLETON pattern behavior.
template<typename T>
struct GlobalSingletonWrapper {
    ~GlobalSingletonWrapper() {
        delete _instance;
    }

    // Access the instance, NOTE: _instance needs to be initialized before usage
    static T &GetInstance() {
        assert(_instance != nullptr && "Not inited Singleton instance!");
        return *_instance;
    }

    static bool IsInited() { return _instance != nullptr; }

protected:
    // Note: Should be used in some deriving class init api.
    static void InitInstance(T *readyInstance) {
        _instance = readyInstance;
    }

    // Note: protected constructor -> class is not able to instantiate outside of child private methods
    GlobalSingletonWrapper() = default;

    static T *_instance;
};

template<typename T>
T *GlobalSingletonWrapper<T>::_instance = nullptr;

using lli = long long int;
static constexpr size_t MB = 1024 * 1024;

// global defined Stack capacity used to store generated moves per thread

#ifdef STACK_SIZE_MOVES

static constexpr size_t DEFAULT_STACK_SIZE = STACK_SIZE_MOVES * MB / sizeof(uint64_t);

#else

static constexpr size_t DEFAULT_STACK_SIZE = 32 * MB / sizeof(uint64_t);

#endif


/* Defines maximal depth of search allowed across the project */
static constexpr int MAX_SEARCH_DEPTH = 128;

/* Defines granularity of score returned by the static evaluation function */
static constexpr int SCORE_GRAIN = 4;

static constexpr uint64_t MSEC_TO_NSEC = 1000 * 1000;

static constexpr int16_t DRAW_SCORE = 0;
static constexpr int16_t SPECIAL_DRAW_SCORE = 0;

static constexpr size_t MAX_MOVES = 256;
static constexpr size_t MAX_QUIET_MOVES = 128;

// -------------------- RESERVED VALUES -------------------------------
static constexpr int RESERVED_SCORE_VALUES = 64;
static constexpr int TIME_STOP_RESERVED_VALUE = std::numeric_limits<int16_t>::max() - 1;
static constexpr int NO_EVAL_RESERVED_VALUE = std::numeric_limits<int16_t>::max() - 2;
static constexpr int EVAL_DRAW_RESERVED_VALUE = std::numeric_limits<int16_t>::max() - 3;
static constexpr int NEGATIVE_INFINITY = std::numeric_limits<int16_t>::min() + RESERVED_SCORE_VALUES + 1;
static constexpr int POSITIVE_INFINITY = std::numeric_limits<int16_t>::max() - RESERVED_SCORE_VALUES;
static constexpr int BEST_MATE_VALUE = NEGATIVE_INFINITY + MAX_SEARCH_DEPTH;
static constexpr int BEST_MATE_VALUE_ABS = -(BEST_MATE_VALUE);

/*
 * Three types of nodes that we can find during the search.
 * To get more information about them, please visit:
 * https://www.chessprogramming.org/Node_Types
 * */

enum NodeType : uint8_t {
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

#endif // USE_ASP_WIN

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
