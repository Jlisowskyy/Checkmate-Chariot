//
// Created by Jlisowskyy on 3/7/24.
//

#include "../include/ThreadManagement/SearchThreadManager.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/ThreadManagement/GameTimeManager.h"

#include <format>

SearchThreadManager::~SearchThreadManager()
{
    for (const auto thread : _threads) delete thread;
}
bool SearchThreadManager::Go(const Board &bd, uint16_t age, const GoInfo &info)
{
    // ensuring only one search is running at a time
    if (_isSearchOn)
        return false;

    // Setting up time guarding parameters
    GameTimeManager::StartSearchManagementAsync(info.timeInfo, static_cast<Color>(bd.MovingColor));

    // Running up the searching worker
    _threads[0] = new std::thread(_threadSearchJob, &bd, &_stacks[0], age, std::min(info.depth, MaxSearchDepth));
    _isSearchOn = true;

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

    _threads[0]->join(); // waiting for the main search thread to finish
    delete _threads[0];
    _threads[0] = nullptr;

    _isSearchOn = false;
}

void SearchThreadManager::_threadSearchJob(const Board *bd, Stack<Move, DefaultStackSize> *s, uint16_t age, int depth)
{
    PackedMove output{};

    BestMoveSearch searcher{*bd, *s, age};
    searcher.IterativeDeepening(&output, depth);

    GlobalLogger << std::format("bestmove {}\n", output.GetLongAlgebraicNotation());
}
