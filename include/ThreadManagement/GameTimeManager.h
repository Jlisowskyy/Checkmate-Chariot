//
// Created by wookie on 5/7/24.
//

#ifndef CHECKMATE_CHARIOT_GAMETIMER_H
#define CHECKMATE_CHARIOT_GAMETIMER_H

#include <chrono>
#include <condition_variable>

#include "../Board.h"
#include "../EngineUtils.h"
#include "../Interface/Logger.h"

class [[maybe_unused]] GameTimeManager
{
    // ------------------------------
    // Class creation - Singleton
    // ------------------------------

    public:
    GameTimeManager()  = delete;
    ~GameTimeManager() = delete;

    GameTimeManager(const GameTimeManager &) = delete;
    GameTimeManager(GameTimeManager &&)      = delete;

    GameTimeManager &operator=(const GameTimeManager &) = delete;
    GameTimeManager &operator=(GameTimeManager &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    /// <summary>
    /// Start the timer asynchronously, on separate thread. This function will periodically check the current time and
    /// update the CurrentTime field.
    /// </summary>
    [[maybe_unused]] static void StartTimerAsync();

    /// @brief Force update the CurrentTime field
    [[maybe_unused]] static std::chrono::time_point<std::chrono::system_clock> ForceUpdateCurrentTime();

    /// <summary>
    /// Start the search management asynchronously, on separate thread. This function will periodically check if the
    /// engine still has time to search for the best move. If the time is up, the variable ShouldStop will be set to
    /// true.
    /// </summary>
    static void
    StartSearchManagementAsync(const GoTimeInfo &tInfo, const Color color, const Board &bd, const uint16_t moveAge);

    static void Restart();
    /* Function sets up stop flag to false and saves time infos for later usage */
    static void StartPonder(const GoTimeInfo &tInfo);

    /* Function starts time management accordingly to previously saved times */
    static void PonderHit(Color color, const Board &bd, uint16_t moveAge);

    /// <summary> Stop the search management thread </summary>
    static void StopSearchManagement();

    static auto GetCurrentTime() { return CurrentTime; }

    static bool GetShouldStop() { return ShouldStop; }

    /// <summary> Calculate the time in milliseconds for a move </summary>
    [[maybe_unused]] static lli CalculateTimeMsPerMove(
        const Board &bd, const lli timeLimitClockMs, const lli timeLimitPerMoveMs, const lli incrementMs,
        const uint16_t moveAge, const Color color
    );

    private:
    /// @See StartTimerAsync
    [[noreturn]] static void _timer_thread();

    /// @See StartSearchManagementAsync
    static void
    _search_management_thread(std::chrono::time_point<std::chrono::system_clock> moveStartTimeMs, lli timeForMoveMs);

    // ------------------------------
    // Class fields
    // ------------------------------

    public:
    /// @brief Time when the timer was started
    static std::chrono::time_point<std::chrono::system_clock> TimeStart;

    /// @brief Current time
    static std::chrono::time_point<std::chrono::system_clock> CurrentTime;

    /// @brief Flag indicating if the search should stop
    static bool ShouldStop;

    static int32_t GeneralExpectedMoves;

    private:
    /// @brief Resolution of the timer in milliseconds
    static constexpr uint32_t resolutionMs = 1;
    static double expectedMoves;
    static double moveCorrection;

    /// @brief Flag indicating if the timer_thread is running
    static bool TimerRunning;

    // Time management constants - used for tuning the time management function
    static constexpr uint32_t averageMovesPerGame = 40;
    static constexpr double adaptationThreshold   = 1.0 / 100;
    static constexpr double adaptationFactor      = 1.0 / 40;
    static constexpr double distribution          = 0.0;

    // Wake on update mechanism
    static std::condition_variable cv;
    static std::mutex mtx;

    static GoTimeInfo _ponderTimes;
};

#endif // CHECKMATE_CHARIOT_GAMETIMER_H
