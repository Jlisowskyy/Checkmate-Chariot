//
// Created by Jlisowskyy on 3/7/24.
//

#include "../include/ThreadManagement/SearchThreadManager.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/ThreadManagement/GameTimeManager.h"

#include <format>

SearchThreadManager::~SearchThreadManager()
{
    // cancel search if is up
    Stop();

    // signal stop
    _shouldStop = true;
    _searchSem.release();

    _threads[MainSearchThreadInd]->join();
    delete _threads[MainSearchThreadInd];
    _threads[MainSearchThreadInd] = nullptr;
}
bool SearchThreadManager::Go(const Board &bd, const GoInfo &info)
{
    // ensuring only one search is running at a time
    if (_isSearchOn)
        return false;

    _isPonderOn = info.isPonderSearch;

    // Setting up time guarding parameters
    if (!info.isPonderSearch)
        GameTimeManager::StartSearchManagementAsync(info.timeInfo, static_cast<Color>(bd.MovingColor), bd, bd.Age);
    else
    {
        GameTimeManager::StartPonder(info.timeInfo);
    }

    // prepare arguments
    _searchArgs.bd    = &bd;
    _searchArgs.depth = info.depth;

    // signal search start
    _searchSem.release();

    // wait for search thread start to prevent races on guard
    _bootupSem.acquire();

    // Signaling success
    return true;
}

bool SearchThreadManager::GoInfinite(const Board &bd)
{
    GoInfo info;
    info.timeInfo = GoTimeInfo::GetInfiniteTime();
    info.depth    = MAX_SEARCH_DEPTH;
    return Go(bd, info);
}

void SearchThreadManager::Stop() const
{
    // avoiding unnecessary actions
    if (!_isSearchOn)
        return;

    // signaling forced abortion
    GameTimeManager::StopSearchManagement();
}

void SearchThreadManager::GoWoutThread(const Board &bd, const GoInfo &info)
{
    static StackType s{};

    GameTimeManager::StartSearchManagementAsync(info.timeInfo, static_cast<Color>(bd.MovingColor), bd, bd.Age);

    PackedMove output{};
    PackedMove ponder{};

    BestMoveSearch searcher{bd, s};
    searcher.IterativeDeepening(&output, &ponder, info.depth);

    GlobalLogger.LogStream << std::format("bestmove {}", output.GetLongAlgebraicNotation())
                           << (ponder.IsEmpty() ? "" : std::format(" ponder {}", ponder.GetLongAlgebraicNotation()))
                           << std::endl;
}

void SearchThreadManager::_passiveThreadSearchJob(
    Stack<Move, DEFAULT_STACK_SIZE> *s, SearchThreadManager::_searchArgs_t *args, bool *guard, const bool *shouldStop,
    std::binary_semaphore *taskSem, std::binary_semaphore *bootup
)
{
    PackedMove output{};
    PackedMove ponder{};

    // be alive until SearchThreadManager is destructed
    while (!*shouldStop)
    {
        // waiting for task
        taskSem->acquire();

        // destruction is being processed
        if (*shouldStop)
        {
            break;
        }

        // Read arguments
        const Board &bd = *(args->bd);
        const int depth = args->depth;

        // harden search status
        *guard = true;
        // signal start command that thread is ready
        bootup->release();

        // run search
        BestMoveSearch searcher{bd, *s};
        searcher.IterativeDeepening(&output, &ponder, depth);

        GlobalLogger.LogStream << std::format("bestmove {}", output.GetLongAlgebraicNotation())
                               << (ponder.IsEmpty() ? "" : std::format(" ponder {}", ponder.GetLongAlgebraicNotation()))
                               << std::endl;

        // harden search status
        *guard = false;
    }
}

SearchThreadManager::SearchThreadManager()
{
    _threads[MainSearchThreadInd] = new std::thread(
        _passiveThreadSearchJob, &GetDefaultStack(), &_searchArgs, &_isSearchOn, &_shouldStop, &_searchSem, &_bootupSem
    );
}
