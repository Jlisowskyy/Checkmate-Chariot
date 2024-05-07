//
// Created by wookie on 5/7/24.
//

#ifndef CHECKMATE_CHARIOT_GAMETIMER_H
#define CHECKMATE_CHARIOT_GAMETIMER_H

#include <chrono>
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

    /// @brief Start timer asynchronously, on separate thread
    [[maybe_unused]] static void StartTimerAsync();
    static std::chrono::time_point<std::chrono::system_clock> ForceUpdateCurrentTime();

    private:
    [[noreturn]] static void _timerThread();

    // ------------------------------
    // Class fields
    // ------------------------------

    public:
    /// @brief Time when the timer was started
    static std::chrono::time_point<std::chrono::system_clock> TimeStart;
    /// @brief Current time
    static std::chrono::time_point<std::chrono::system_clock> CurrentTime;

    private:
    /// @brief Resolution of the timer in milliseconds
    static constexpr uint32_t resolutionMs = 1;
};

#endif // CHECKMATE_CHARIOT_GAMETIMER_H
