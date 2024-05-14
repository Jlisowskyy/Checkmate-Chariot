//
// Created by Jlisowskyy on 3/7/24.
//

#include "../include/ThreadManagement/SearchThreadManager.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/ThreadManagement/GameTimeManager.h"

#include <format>

SearchThreadManager::~SearchThreadManager() { Consolidate(); }
bool SearchThreadManager::Go(const Board &bd, uint16_t age, const GoInfo &info)
{
    // ensuring only one search is running at a time
    if (_isSearchOn)
        return false;

    // Setting up time guarding parameters
    GameTimeManager::StartSearchManagementAsync(info.timeInfo, static_cast<Color>(bd.MovingColor), bd, age);

    // Running up the searching worker
    _threads[MainSearchThreadInd] = new std::thread(
        _threadSearchJob, &bd, &_stacks[MainSearchThreadInd], &_isSearchOn, age, std::min(info.depth, MaxSearchDepth)
    );
    WrapTraceMsgInfo("Search thread started");

    // Signaling success
    return true;
}

bool SearchThreadManager::GoInfinite(const Board &bd, uint16_t age)
{
    GoInfo info;
    info.timeInfo = GoTimeInfo::GetInfiniteTime();
    info.depth    = MaxSearchDepth;
    return Go(bd, age, info);
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
    const Board *bd, Stack<Move, DefaultStackSize> *s, bool *guard, uint16_t age, int depth
)
{
    PackedMove output{};

    *guard = true;
    BestMoveSearch searcher{*bd, *s, age};
    searcher.IterativeDeepening(&output, depth);

    GlobalLogger.LogStream << std::format("bestmove {}", output.GetLongAlgebraicNotation()) << std::endl;
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
