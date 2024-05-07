//
// Created by wookie on 5/7/24.
//

#include <chrono>
#include <thread>

#include "../include/ThreadManagement/GameTimeManager.h"

/// @brief Start timer asynchronously, on separate thread
[[maybe_unused]] void GameTimeManager::StartTimerAsync()
{
    TimeStart = std::chrono::system_clock::now();
    std::thread timerThread(_timerThread);
    timerThread.detach(); // detach the thread to avoid memory leak
}

/// @brief Check the current time with a given resolution and update the CurrentTime field
[[noreturn]] void GameTimeManager::_timerThread()
{
    while (true)
    {
        CurrentTime = std::chrono::system_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(resolutionMs));
    }
}

/// @brief Force update the CurrentTime field
std::chrono::time_point<std::chrono::system_clock> GameTimeManager::ForceUpdateCurrentTime()
{
    return CurrentTime = std::chrono::system_clock::now();
}
