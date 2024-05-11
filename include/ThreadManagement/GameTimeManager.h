//
// Created by wookie on 5/7/24.
//

#ifndef CHECKMATE_CHARIOT_GAMETIMER_H
#define CHECKMATE_CHARIOT_GAMETIMER_H

#include <chrono>
#include <condition_variable>

#include "../Board.h"
#include "../EngineUtils.h"

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
    static void StartSearchManagementAsync(const GoTimeInfo &tInfo, const Color color, const Board& board);

    /// <summary> Stop the search management thread </summary>
    static void StopSearchManagement();

    private:
    /// @See StartTimerAsync
    [[noreturn]] static void _timer_thread();

    /// @See StartSearchManagementAsync
    static void _search_management_thread(
        const GoTimeInfo &tInfo, const Color color, const lli timeForMoveMs);

    /// <summary> Calculate the time in milliseconds for a move </summary>
    [[maybe_unused]] static lli CalculateTimeMsForMove(const Board &bd, const lli timeLeftBoardMs);

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

    private:
    /// @brief Resolution of the timer in milliseconds
    static constexpr uint32_t resolutionMs = 1;

    /// @brief Flag indicating if the timer_thread is running
    static bool TimerRunning;

    // Wake on update mechanism
    static std::condition_variable cv;
    static std::mutex mtx;
};

#endif // CHECKMATE_CHARIOT_GAMETIMER_H