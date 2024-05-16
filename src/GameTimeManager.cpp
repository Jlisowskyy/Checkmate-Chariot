//
// Created by wookie on 5/7/24.
//

#include <cassert>
#include <limits>
#include <thread>

#include "../include/Board.h"
#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/ThreadManagement/GameTimeManager.h"
#include "../include/ThreadManagement/GameTimeManagerUtils.h"

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

void GameTimeManager::StartSearchManagementAsync(
    const GoTimeInfo &tInfo, const Color color, const Board &bd, const uint16_t moveAge
)
{
    assert(TimerRunning && "Timer must be running"); // Timer must be running

    // Set the beginning of the move to the current time
    auto moveStartTimeMs = CurrentTime;

    ShouldStop = false;

    auto [timeLimitClockMs, timeLimitPerMoveMs, incrementMs] = GameTimeManagerUtils::ParseGoTimeInfo(tInfo, color);

    // If both time limits are not set, then there is no time limit
    if (timeLimitClockMs == GoTimeInfo::Infinite && timeLimitPerMoveMs == GoTimeInfo::Infinite)
    {
        return;
    }

    lli timeForMoveMs = timeLimitPerMoveMs; // Default to max time per move
    if (timeLimitClockMs != GoTimeInfo::Infinite)
    { // If there is a time limit on the clock calculate the time for the move
        timeForMoveMs = CalculateTimeMsPerMove(bd, timeLimitClockMs, timeLimitPerMoveMs, incrementMs, moveAge);
    }

    // const lli timeForMoveMs = 0;

    // time limit
    std::thread searchManagementThread(_search_management_thread, moveStartTimeMs, timeForMoveMs);
    searchManagementThread.detach();
}

void GameTimeManager::StopSearchManagement() { ShouldStop = true; }

void GameTimeManager::_search_management_thread(
    const std::chrono::time_point<std::chrono::system_clock> moveStartTimeMs, const lli timeForMoveMs
)
{
    const auto moveStopTimeMs = moveStartTimeMs + std::chrono::milliseconds(timeForMoveMs);

    while (!ShouldStop)
    {
        {
            // Wait for update of the current time
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock);
        }

        if (CurrentTime >= moveStopTimeMs)
        {
            ShouldStop = true;
        }
    }
}

lli GameTimeManager::CalculateTimeMsPerMove(
    const Board &bd, const lli timeLimitClockMs, const lli timeLimitPerMoveMs, const lli incrementMs,
    const uint16_t moveAge
)
{
    /*
     * Main ideas:
     * 1. In calculations, mitigate the fact that the time is constantly decreasing
     * 2. Try to predict the number of moves the game will take
     * 3. Give weights to moves depending on the stage of the game
     *
     * 1: (for simplicity, assuming the the moves are equally important)
     * if we expect the game to end in 40 moves, we take 1/40 of the time.
     * if we expect the game to end in 10 moves, we take 1/10 of the time. etc
     *
     * This ensures that even though the total time on the clock is decreasing, we are taking the same amount of
     * time per move (time decrease mitigation in calculations)
     *
     * 2: (still assuming the moves are equally important)
     * As the game progresses, we can keep track of the derivative of the phase of the game. If it's progressing
     * very slowly, we predict that the game will take longer, and adjust the expected 'num of moves to game finish'
     *
     * 3: Moves have a weight distribution depending on the stage of the game
     * early-game and late-game are less important. Mid-game is the most important
     *
     * The percentage of time to be taken needs to be calculated relative to the weight of the move.
     * But the weight of the move has to be calculated in a smart way, so that slicing time using it, will still yield
     * times that follow our model of distribution and importance (adhere to 1) and adhere to (2)
     *
     */

    /*
     * The equation for the time per move is:
     * ---------------------------------- Equation ----------------------------------
     *
     *                                                 scale
     *                    f(x) = -( x ⋅(x - xlimit) ) ⋅----- + xmin
     *                                                   q
     *
     * --------------------------------- Variables ----------------------------------
     * x - game stage
     * xlimit - max game stage
     * xmin - min time per move
     * scale - this is a scaling factor for the parabola
     *
     *             2
     *     (xlimit)
     * q = --------- - is the peak of the quadratic function, used to normalize it to
     *         4       [0, 1] and then scale it
     *
     * ------------------------------------------------------------------------------
     */

    /*
     * ------------------------ The function should satisfy -------------------------
     *
     *                    b
     *               em  /  /                       scale        \
     *             ----- |  | -( x ⋅(x - xlimit) ) ⋅----- + xmin |  dx = timeleft
     *             b - a /  \                         q          /
     *                    a
     *
     * --------------------------------- Variables ----------------------------------
     *
     * a - current game stage
     * b = xlimit - max game stage
     * em - expected moves to game finish
     * timeleft - time left on the clock
     *
     * --------------------------------- Reasoning ----------------------------------
     *
     * 1. The integral of the function divided by the interval yields it's average
     * value E[f(x)].
     * 2. We probe the function in an almost uniform manner, and we want the sum of
     * those probes (em - expected moves) to be equal to the time left on the clock.
     * 3. Adjusting the scale will enforce that this is true.
     */

    /*
     * ------------------------------ Solve for scale -------------------------------
     * ---------------------------------- Equation ----------------------------------
     *
     *                              2
     *                           3 b  (- timeleft + em × xmin)
     *                  -scale = ----------------------------
     *                                        2         2
     *                              2 em (- 2a  + ab + b ) # TODO: Wrong formula
     *
     * ------------------------------------------------------------------------------
     */

    constexpr int32_t minGameStage = 0;
    constexpr int32_t maxGameStage = (int32_t)ConstexprMath::sqrt(std::numeric_limits<int32_t>::max()) / 2;

    const int32_t a    = BoardEvaluator::InterpGameStage(bd, minGameStage, maxGameStage);
    constexpr double b = maxGameStage;
    const int32_t em   = (int32_t)(50 - moveAge / 2);
    // const int32_t em = (int32_t)(47 * ((b - a)/b) + 3); // TODO: this is a temporary value and should be calculated
    const int32_t xmin = (int32_t) (((double)timeLimitClockMs / ((double) em)) / 4);
    // TODO: this is a temporary value and should be calculated

    GlobalLogger.TraceStream << std::format("[ INFO ] Calculating time for this move \n");
    GlobalLogger.TraceStream << std::format("[ INFO ] Game stage: {}/{} \n", a, b);
    GlobalLogger.TraceStream << std::format("[ INFO ] Expected moves to game finish: {} \n", em);
    GlobalLogger.TraceStream << std::format("[ INFO ] MinMoveTimeMs: {} \n", xmin);

    double timeForMoveMs = 0;
    if (timeLimitClockMs <= 10)
    {
        // For really short time limits just divide the time left by the expected moves
        timeForMoveMs = (double)timeLimitClockMs / (double)em;
        timeForMoveMs = timeForMoveMs + incrementMs < 1 ? 1 : timeForMoveMs;
        GlobalLogger.TraceStream << std::format(
            "[ INFO ] Time is really short, calculating with: 'time left divided by expected moves': {} \n",
            timeForMoveMs
        );
    }
    else
    {
        // Calculate the scale
        const double q = (double)(b * b) / 4;

        const double scale = ( (double)(xmin * (b-a) - ((double)timeLimitClockMs * (b-a) / em) ) * q)
                 / ( (double)(b*b*b - a*a*a) / 3 - (double)((b * b - a * a) / 2) * b );

        //const double scale =
        //    -(3 * b * b * ((double)-timeLimitClockMs + (em * xmin))) / (2 * em * (a - b) * (2 * a + b));

        assert(scale >= 0 && "Scale must be positive");
        GlobalLogger.TraceStream << std::format("[ INFO ] Scale: {} \n", scale);

        // Evaluate the quadratic function
        timeForMoveMs = (-(a * (a - b)) * scale / q) + xmin + (double)incrementMs;
    }

    timeForMoveMs = std::max(timeForMoveMs, (double)xmin);
    timeForMoveMs = std::min(timeForMoveMs, (double)timeLimitPerMoveMs);
    const lli ans = (lli)timeForMoveMs;
    GlobalLogger.TraceStream << std::format("[ INFO ] Time calculated for this move: {}", ans) << std::endl;

    assert(ans >= 0 && "Time for move must be positive");
    assert(ans <= timeLimitPerMoveMs && "Time for move must be less than the time limit per move");
    assert(ans <= timeLimitClockMs && "Time for move must be less than the time limit on the clock");
    assert(ans >= xmin && "Time for move must be greater or equal to the min time per move");
    assert(ans > 0 && "Time for move must be greater than 0");

    return ans;
}
