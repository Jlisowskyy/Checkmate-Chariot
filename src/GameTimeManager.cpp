//
// Created by wookie on 5/7/24.
//

#include <cassert>
#include <thread>

#include "../include/ThreadManagement/GameTimeManager.h"

// Static fields initialization
bool GameTimeManager::TimerRunning = false;
bool GameTimeManager::ShouldStop   = false;
std::chrono::time_point<std::chrono::system_clock> GameTimeManager::TimeStart;
std::chrono::time_point<std::chrono::system_clock> GameTimeManager::CurrentTime;
std::mutex GameTimeManager::mtx;
std::condition_variable GameTimeManager::cv;

void GameTimeManager::StartTimerAsync()
{
    if (TimerRunning)
        return; // Timer is already running

    TimerRunning = true;
    TimeStart    = std::chrono::system_clock::now();
    CurrentTime  = TimeStart;
    std::thread timerThread(_timer_thread);
    timerThread.detach(); // detach the thread to avoid memory leak
}

void GameTimeManager::_timer_thread()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            CurrentTime = std::chrono::system_clock::now();
        }
        cv.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(resolutionMs));
    }
}

[[maybe_unused]] std::chrono::time_point<std::chrono::system_clock> GameTimeManager::ForceUpdateCurrentTime()
{
    return CurrentTime = std::chrono::system_clock::now();
}

void GameTimeManager::StartSearchManagementAsync(const GoTimeInfo &tInfo, const Color &color)
{
    assert(TimerRunning && "Timer must be running"); // Timer must be running
    std::thread searchManagementThread(_search_management_thread, tInfo, color);
    searchManagementThread.detach();
}

void GameTimeManager::_search_management_thread(const GoTimeInfo &tInfo, const Color &color)
{
    lli timeLeftMs      = color == Color::WHITE ? tInfo.wTime == GoTimeInfo::NotSet ? GoTimeInfo::Infinite : tInfo.wTime
                          : tInfo.bTime == GoTimeInfo::NotSet ? GoTimeInfo::Infinite
                                                              : tInfo.bTime;
    lli moveTimeLimitMs = tInfo.moveTime == GoTimeInfo::NotSet ? GoTimeInfo::Infinite : tInfo.moveTime;
    assert(
        (timeLeftMs != GoTimeInfo::Infinite || moveTimeLimitMs != GoTimeInfo::Infinite) &&
        "At least one of the time limits must be finite"
    );

    const auto stopTimeCloc = CurrentTime + std::chrono::milliseconds(moveTimeLimitMs);

    ShouldStop = false;
    while (!ShouldStop)
    {
        {
            // Wait for update of the current time
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock);
        }

        if (CurrentTime >= stopTimeCloc)
        {
            ShouldStop = true;
        }
    }
}

lli GameTimeManager::CalculateMsForMove()
{
    /// @todo Implement this function
    /// Should take in the account the time left for the move and the stage of the game.
    /// And return the time (in milliseconds) that the engine should spend on the move.
    return 0;
}
