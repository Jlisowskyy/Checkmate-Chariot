//
// Created by Jlisowskyy on 7/22/24.
//

#include "../include/TunableParameters.h"

template<> GlobalParametersList* GlobalSingletonWrapper<GlobalParametersList>::_instance = nullptr;

DEFINE_TUNABLE_PARAM(int, FULL_DEPTH_FACTOR, 4);
DEFINE_TUNABLE_PARAM(int, CHECK_EXTENSION_PV_NODE, FULL_DEPTH_FACTOR::Get());
DEFINE_TUNABLE_PARAM(int, CHECK_EXTENSION, FULL_DEPTH_FACTOR::Get() / 2);
DEFINE_TUNABLE_PARAM(int, PV_EXTENSION, FULL_DEPTH_FACTOR::Get() / 2);
DEFINE_TUNABLE_PARAM(int, EVEN_EXCHANGE_EXTENSION_PV_NODE, FULL_DEPTH_FACTOR::Get());
DEFINE_TUNABLE_PARAM(int, EVEN_EXCHANGE_EXTENSION, FULL_DEPTH_FACTOR::Get() / 2);
DEFINE_TUNABLE_PARAM(int, SINGULAR_EXTENSION_DEPTH_PROBE_LIMIT, 3);
DEFINE_TUNABLE_PARAM(int, SINGULAR_EXTENSION_MIN_DEPTH, 4);
DEFINE_TUNABLE_PARAM(int, SINGULAR_EXTENSION, FULL_DEPTH_FACTOR::Get() * 2);
DEFINE_TUNABLE_PARAM(int, SINGULAR_EXTENSION_BETA_COEF, 16 / SCORE_GRAIN);
DEFINE_TUNABLE_PARAM(int, SINGULAR_EXTENSION_BETA_DIV, 1);

DEFINE_TUNABLE_PARAM(int, LMR_MIN_DEPTH, 3 * FULL_DEPTH_FACTOR::Get());
DEFINE_TUNABLE_PARAM(int, SEE_GOOD_MOVE_BOUNDARY, -115 / SCORE_GRAIN);
DEFINE_TUNABLE_PARAM(int, NO_TT_MOVE_REDUCTION, 1 * FULL_DEPTH_FACTOR::Get());
DEFINE_TUNABLE_PARAM(int, END_GAME_PHASE, 64);
DEFINE_TUNABLE_PARAM(int, IID_MIN_DEPTH_PLY_DEPTH, 5);
DEFINE_TUNABLE_PARAM(int, IID_REDUCTION, 3 * FULL_DEPTH_FACTOR::Get());
DEFINE_TUNABLE_PARAM(int, ASP_WND_MIN_DEPTH, 7);
DEFINE_TUNABLE_PARAM(int, INITIAL_ASP_WINDOW_DELTA, 3);
DEFINE_TUNABLE_PARAM(int, MAX_ASP_WINDOW_RETRIES, 4);
DEFINE_TUNABLE_PARAM(int, LMR_TT_ALL_NODE_REDUCTION_MAX_DEPTH_DIFF, 5);

DEFINE_TUNABLE_PARAM(int, DELTA_PRUNING_SAFETY_MARGIN, (115 + 115) / SCORE_GRAIN);
DEFINE_TUNABLE_PARAM(int, DELTA_PRUNING_PROMO, (1000 - 115) / SCORE_GRAIN);
DEFINE_TUNABLE_PARAM(int, SEE_BASED_PRUNING_GOOD_MOVE_COEF, 8);
DEFINE_TUNABLE_PARAM(int, ENABLE_RAZORING, 1);
DEFINE_TUNABLE_PARAM(int, RAZORING_DEPTH, 3);
DEFINE_TUNABLE_PARAM(int, RAZORING_MARGIN, (900 + (900 - 100) + 500 + 100) / SCORE_GRAIN);

DEFINE_TUNABLE_PARAM(int, HISTORY_TABLE_POINTS_LIMIT, 16*1024);
DEFINE_TUNABLE_PARAM(int, HISTORY_BONUS_COEF, 2);
DEFINE_TUNABLE_PARAM(int, HISTORY_PENALTY_COEF, 1);
DEFINE_TUNABLE_PARAM(int, HISTORY_PENALTY_BIAS, 1);
DEFINE_TUNABLE_PARAM(int, HISTORY_SCALE_DOWN_FACTOR, 4);
DEFINE_TUNABLE_PARAM(int, LMR_GOOD_HISTORY_REDUCTION_DIV, HISTORY_TABLE_POINTS_LIMIT::Get() / 2);

DEFINE_TUNABLE_PARAM(int, COUNTER_MOVE_TABLE_PRIZE, 600);
DEFINE_TUNABLE_PARAM(int, MOVE_SORT_PROMO_CHECK, 100);
DEFINE_TUNABLE_PARAM(int, MOVE_SORT_CAPTURE_COEF, 1);
DEFINE_TUNABLE_PARAM(int, MOVE_SORT_CAPTURE_DIV, 8);
DEFINE_TUNABLE_PARAM(int, HISTORY_BONUS_BIAS, 0);
DEFINE_TUNABLE_PARAM(int, MOVE_SORT_QUIET_CHECK, HISTORY_TABLE_POINTS_LIMIT::Get() / 8);
DEFINE_TUNABLE_PARAM(int, MOVE_SORT_GOOD_QUIET_SCORE, 50);
DEFINE_TUNABLE_PARAM(int, MOVE_SORT_QUIETS_PAWN_EVASION_BONUS, 50);
DEFINE_TUNABLE_PARAM(int, MOVE_SORT_QUIETS_PAWN_DANGER_PENALTY, -50);
DEFINE_TUNABLE_PARAM(uint16_t, QUIESENCE_AGE_DIFF_REPLACE, 16);
DEFINE_TUNABLE_PARAM(uint16_t, DEFAULT_AGE_DIFF_REPLACE, 10);

DEFINE_TUNABLE_PARAM(int, CONT_TABLE_SCALE_DOWN_FACTOR, 1);