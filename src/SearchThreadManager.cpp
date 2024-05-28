//
// Created by Jlisowskyy on 3/7/24.
//

#include "../include/ThreadManagement/SearchThreadManager.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/ThreadManagement/GameTimeManager.h"

#include <format>

SearchThreadManager::~SearchThreadManager()
{
    Stop();
    Consolidate();
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

    // Running up the searching worker
    _threads[MainSearchThreadInd] = new std::thread(
        _threadSearchJob, &bd, &_stacks[MainSearchThreadInd], &_isSearchOn,
        std::min(info.depth, MAX_SEARCH_DEPTH)
    );
    WrapTraceMsgInfo("Search thread started");

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

void SearchThreadManager::Stop()
{
    // avoiding unnecessary actions
    if (!_isSearchOn)
        return;

    // signaling forced abortion
    GameTimeManager::StopSearchManagement();

    _threads[MainSearchThreadInd]->join(); // waiting for the main search thread to finish
    delete _threads[MainSearchThreadInd];
    _threads[MainSearchThreadInd] = nullptr;
    WrapTraceMsgInfo("Search thread stopped successfully");
}

void SearchThreadManager::_threadSearchJob(
    const Board *bd, Stack<Move, DEFAULT_STACK_SIZE> *s, bool *guard, int depth
)
{
    PackedMove output{};
    PackedMove ponder{};

    *guard = true;
    BestMoveSearch searcher{*bd, *s};
    searcher.IterativeDeepening(&output, &ponder, depth);

    GlobalLogger.LogStream << std::format("bestmove {}", output.GetLongAlgebraicNotation())
                           << (ponder.IsEmpty() ? "" : std::format(" ponder {}", ponder.GetLongAlgebraicNotation()))
                           << std::endl;

    *guard = false;
}
void SearchThreadManager::Consolidate()
{
    if (_threads[MainSearchThreadInd] != nullptr)
    {
        _threads[MainSearchThreadInd]->join();
        delete _threads[MainSearchThreadInd];
        _threads[MainSearchThreadInd] = nullptr;
    }

    WrapTraceMsgInfo("Thread manager consolidated successfully");
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
