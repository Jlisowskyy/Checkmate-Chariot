//
// Created by Jlisowskyy on 7/19/24.
//

#ifndef CHECKMATE_CHARIOT_TUNABLEPARAMETERS_H
#define CHECKMATE_CHARIOT_TUNABLEPARAMETERS_H

#include "CompilationConstants.h"

#include <string>
#include <unordered_map>
#include <charconv>
#include <iostream>

// Determines whether we use variables or const values to parameters
#ifdef ALLOW_TESTING_VARIABLES

#define TEST_VARIABLE_MOD
#define INIT_PARAM(defaultValue)
static constexpr bool AllowTestVariable = true;

#else

#define TEST_VARIABLE_MOD constexpr
#define INIT_PARAM(defaultValue)  = (defaultValue)
static constexpr bool AllowTestVariable = false;

#endif // ALLOW_TESTING_VARIABLES

/* Simple Interface struct defining operations allowed on parameter stored in global prameters list */
struct TunableParameterIntf {
    virtual ~TunableParameterIntf()      = default;
    virtual void Set(const std::string&) = 0;
    virtual void Display() = 0;
};

/* Simple class defining the interface operations mentioned above */
template<typename ParamT>
struct TunableParameter final : TunableParameterIntf {
    explicit TunableParameter(ParamT &param, const char *name) : _name(name), _param(param) {}
    ~TunableParameter() override = default;

    void Set(const std::string &str) override {
        if (ParamT rv{}; std::from_chars(str.c_str(), str.c_str() + str.size(), rv).ec != std::errc::invalid_argument)\
                _param = rv;
    }

    void Display() override  {
        std::cout << _name << " : " << _param << std::endl;
    }

private:
    const char* _name;
    ParamT& _param;
};

/* Singleton class storing interfaces of all defined parameters */
struct GlobalParametersList : GlobalSingletonWrapper<GlobalParametersList>{
    ~GlobalParametersList() = default;

    TunableParameterIntf* GetParameter(const std::string& param)
    {
        if (_params.contains(param))
            return _params[param];
        return nullptr;
    }

    void AddEntry(const std::string& param, TunableParameterIntf* wrapper)
    {
        if (!_params.contains(param))
            _params[param] = wrapper;
    }

    void DisplayAll() const {
        for (const auto& [k, v] : _params)
            v->Display();
    }

    static void Init() {
        if (!IsInited())
            InitInstance(new GlobalParametersList());
    }

private:
    GlobalParametersList() = default;

    std::unordered_map<std::string, TunableParameterIntf*> _params{};
};

#define DECLARE_TUNABLE_PARAM(ParamT, name, defaultValue) \
struct name {    \
    name() = delete;        \
    ~name() = delete;   \
    static TEST_VARIABLE_MOD ParamT Get() { return _param; } \
    static TEST_VARIABLE_MOD ParamT _param INIT_PARAM(defaultValue);\
}

// Determines whether we use variables or const values to parameters
#ifdef ALLOW_TESTING_VARIABLES

#define DEFINE_TUNABLE_PARAM(type, name, defaultValue) \
    type name::_param = []() {    \
        if (!GlobalParametersList::IsInited())  \
            GlobalParametersList::Init();   \
        GlobalParametersList::GetInstance().AddEntry(#name, new TunableParameter(name::_param, #name)); \
        return defaultValue;    \
    }()

#else

#define DEFINE_TUNABLE_PARAM(type, name, defaultValue)

#endif // ALLOW_TESTING_VARIABLES

// ------------------------- EXTENSIONS ------------------------------

DECLARE_TUNABLE_PARAM(int, FULL_DEPTH_FACTOR, 4);
DECLARE_TUNABLE_PARAM(int, CHECK_EXTENSION_PV_NODE, FULL_DEPTH_FACTOR::Get());
DECLARE_TUNABLE_PARAM(int, CHECK_EXTENSION, FULL_DEPTH_FACTOR::Get() / 2);
DECLARE_TUNABLE_PARAM(int, PV_EXTENSION, FULL_DEPTH_FACTOR::Get() / 2);
DECLARE_TUNABLE_PARAM(int, EVEN_EXCHANGE_EXTENSION_PV_NODE, FULL_DEPTH_FACTOR::Get());
DECLARE_TUNABLE_PARAM(int, EVEN_EXCHANGE_EXTENSION, FULL_DEPTH_FACTOR::Get() / 2);


// ------------------------------ REDUCTIONS --------------------------------------

DECLARE_TUNABLE_PARAM(int, LMR_MIN_DEPTH, 3 * FULL_DEPTH_FACTOR::Get());

// limits maximal extensions inside the search tree to not overdo
constexpr bool ShouldExtend(const int ply, const int rootDepth) { return ply < 2 * rootDepth; }

inline INLINE int CalcReductions(const int depth, const int moveCount, const int scoreDelta)
{
    constexpr double SCALE_COEF = 400;

    // Base reduction calculating reduction base for expected fail nodes
    const int reductionBase =
            static_cast<int>(SCALE_COEF * std::sqrt(static_cast<double>(depth - 1)) * std::sqrt(static_cast<double>(moveCount - 1))) /  1024;

    // Decrease reduction for PV-Nodes we are at full window search
    return (scoreDelta != 1 ? 2 * reductionBase / 3 : reductionBase) * 4;
}

// value below which SEE capture is considered bad
DECLARE_TUNABLE_PARAM(int, SEE_GOOD_MOVE_BOUNDARY, -115 / SCORE_GRAIN);

// value defines how much we reduce the search when no TT move is available
DECLARE_TUNABLE_PARAM(int, NO_TT_MOVE_REDUCTION, 1 * FULL_DEPTH_FACTOR::Get());

// value of phase below game is considering to be an end-game
DECLARE_TUNABLE_PARAM(int, END_GAME_PHASE, 64);

/* Depth from which Internal Iterative Deepening (IID) is used */
DECLARE_TUNABLE_PARAM(int, IID_MIN_DEPTH_PLY_DEPTH, 5);

/* Ply reduction for IID case*/
DECLARE_TUNABLE_PARAM(int, IID_REDUCTION, 3 * FULL_DEPTH_FACTOR::Get());

/* Minimal depth from which Aspiration Windows are used*/
DECLARE_TUNABLE_PARAM(int, ASP_WND_MIN_DEPTH, 7);

// Initial Aspiration Window Delta its cp value is equal to INITIAL_ASP_WINDOW_DELTA * BoardEvaluator::ScoreGrain
// (probably 8) ~= 48
DECLARE_TUNABLE_PARAM(int, INITIAL_ASP_WINDOW_DELTA, 3);
DECLARE_TUNABLE_PARAM(int, MAX_ASP_WINDOW_RETRIES, 4);
DECLARE_TUNABLE_PARAM(int, LMR_TT_ALL_NODE_REDUCTION_MAX_DEPTH_DIFF, 5);

// ------------------------- PRUNING ----------------------------------------------------

// average pawn value + some part of average pawn
DECLARE_TUNABLE_PARAM(int, DELTA_PRUNING_SAFETY_MARGIN, (115 + 115) / SCORE_GRAIN);

// average queen value - average pawn value
DECLARE_TUNABLE_PARAM(int, DELTA_PRUNING_PROMO, (1000 - 115) / SCORE_GRAIN);

DECLARE_TUNABLE_PARAM(int, SEE_BASED_PRUNING_GOOD_MOVE_COEF, 8);

DECLARE_TUNABLE_PARAM(int, ENABLE_RAZORING, 0);

DECLARE_TUNABLE_PARAM(int, RAZORING_DEPTH, 3);

// queen killed + promoted + rook killed + safety margin
DECLARE_TUNABLE_PARAM(int, RAZORING_MARGIN, (900 + (900 - 100) + 500 + 100) / SCORE_GRAIN);

// ---------------------------------------------------------------------------------------


// -------------------------- HISTORY TABLE -----------------------------------------------

// Defines maximal points stored inside the history table during the search
DECLARE_TUNABLE_PARAM(int, HISTORY_TABLE_POINTS_LIMIT, 16*1024);

// Bonus linear function parameters
DECLARE_TUNABLE_PARAM(int, HISTORY_BONUS_COEF, 2);
DECLARE_TUNABLE_PARAM(int, HISTORY_BONUS_BIAS, 0);

// Penalty linear function parameters
DECLARE_TUNABLE_PARAM(int, HISTORY_PENALTY_COEF, 1);
DECLARE_TUNABLE_PARAM(int, HISTORY_PENALTY_BIAS, 1);

DECLARE_TUNABLE_PARAM(int, HISTORY_SCALE_DOWN_FACTOR, 4);

DECLARE_TUNABLE_PARAM(int, LMR_GOOD_HISTORY_REDUCTION_DIV, HISTORY_TABLE_POINTS_LIMIT::Get() / 2);

// -----------------------------------------------------------------------------------------

// --------------------------- Counter Move Table ------------------------------------------

DECLARE_TUNABLE_PARAM(int, COUNTER_MOVE_TABLE_PRIZE, 600);

// -----------------------------------------------------------------------------------------

// ---------------------------- Cont history tables ----------------------------------------

static constexpr int CONT_HISTORY_SCORE_TABLES_WRITE_COUNT = 4;
static constexpr int CONT_HISTORY_SCORE_TABLES_READ_COUNT = 3;

DECLARE_TUNABLE_PARAM(int, CONT_TABLE__TABLE_POINTS_LIMIT, 16*1024);
DECLARE_TUNABLE_PARAM(int, CONT_TABLE_SCALE_DOWN_FACTOR, 4);

// -----------------------------------------------------------------------------------------

// ---------------------------- Direct Move Sort -------------------------------------------

DECLARE_TUNABLE_PARAM(int, MOVE_SORT_PROMO_CHECK, 100);

DECLARE_TUNABLE_PARAM(int, MOVE_SORT_CAPTURE_COEF, 1);
DECLARE_TUNABLE_PARAM(int, MOVE_SORT_CAPTURE_DIV, 8);

DECLARE_TUNABLE_PARAM(int, MOVE_SORT_QUIET_CHECK, HISTORY_TABLE_POINTS_LIMIT::Get() / 8);

DECLARE_TUNABLE_PARAM(int, MOVE_SORT_GOOD_QUIET_SCORE, 50);

DECLARE_TUNABLE_PARAM(int, MOVE_SORT_QUIETS_PAWN_EVASION_BONUS, 50);
DECLARE_TUNABLE_PARAM(int, MOVE_SORT_QUIETS_PAWN_DANGER_PENALTY, -50);

// -----------------------------------------------------------------------------------------

// ---------------------------- SINGULAR EXTENSIONS ----------------------------------------

DECLARE_TUNABLE_PARAM(int, SINGULAR_EXTENSION_DEPTH_PROBE_LIMIT, 3);
DECLARE_TUNABLE_PARAM(int, SINGULAR_EXTENSION_MIN_DEPTH, 4);
DECLARE_TUNABLE_PARAM(int, SINGULAR_EXTENSION, FULL_DEPTH_FACTOR::Get() * 2);
DECLARE_TUNABLE_PARAM(int, SINGULAR_EXTENSION_BETA_COEF, 16 / SCORE_GRAIN);
DECLARE_TUNABLE_PARAM(int, SINGULAR_EXTENSION_BETA_DIV, 1);

// -----------------------------------------------------------------------------------------

// ---------------------------- Transpo Table params ---------------------------------------

DECLARE_TUNABLE_PARAM(uint16_t, QUIESENCE_AGE_DIFF_REPLACE, 16);
DECLARE_TUNABLE_PARAM(uint16_t, DEFAULT_AGE_DIFF_REPLACE, 10);

// -----------------------------------------------------------------------------------------

#endif //CHECKMATE_CHARIOT_TUNABLEPARAMETERS_H
