//
// Created by Jlisowskyy on 3/7/24.
//

#include "../include/ThreadManagement/SearchThreadManager.h"
#include "../include/Search/BestMoveSearch.h"

#ifdef __unix__

#include <csignal>

#elif defined(__WIN32__)

#include <Windows.h>

#endif // __unix__

SearchThreadManager::~SearchThreadManager()
{
    for (const auto thread : _threads) delete thread;
}

bool SearchThreadManager::GoInfinite(const Board &bd, uint16_t age)
{
    if (_isSearchOn)
        return false;

    _threads[0] = new std::thread(_threadSearchJob, &bd, &_stacks[0], &_searchResult, MaxSearchDepth, age);
    _isSearchOn = true;

    return true;
}

std::string SearchThreadManager::Stop()
{
    if (!_isSearchOn)
        return "";

    _cancelThread(0);
    _isSearchOn = false;
    return _searchResult.GetLongAlgebraicNotation();
}

std::string SearchThreadManager::GoMoveTime(const Board &bd, long long msecs, uint16_t age)
{
    if (_isSearchOn)
        return "";

    GoInfinite(bd, age);
    std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
    return Stop();
}

std::string SearchThreadManager::GoDepth(const Board &bd, int depth, uint16_t age)
{
    if (_isSearchOn)
        return "";

    _threads[0] =
        new std::thread(_threadSearchJob, &bd, &_stacks[0], &_searchResult, std::min(depth, MaxSearchDepth), age);
    _threads[0]->join();

    delete _threads[0];
    _threads[0] = nullptr;
    _stacks[0].Clear();

    return _searchResult.GetLongAlgebraicNotation();
}

void SearchThreadManager::_threadSearchJob(
    const Board *bd, Stack<Move, DefaultStackSize> *s, PackedMove *output, const int depth, const uint16_t age
)
{
#ifdef __unix__
    if (_setHandler(_sigusr1Exit, SIGUSR1))
        exit(EXIT_FAILURE);
#endif // __unix__

    BestMoveSearch searcher{*bd, *s, age};
    searcher.IterativeDeepening(output, depth);
}

void SearchThreadManager::_cancelThread(const size_t threadInd)
{
    const auto tid = _threads[threadInd]->native_handle();
#ifdef __unix__
    pthread_kill(tid, SIGUSR1);
#elif defined(__WIN32__)
    TerminateThread(tid, 0);
#endif

    _threads[threadInd]->join();
    delete _threads[threadInd];
    _threads[threadInd] = nullptr;

    _stacks[threadInd].Clear();
}

#ifdef __unix__

int SearchThreadManager::_setHandler(void (*f)(int), int sigNo)
{
    struct sigaction act = {};
    act.sa_handler       = f;
    if (-1 == sigaction(sigNo, &act, nullptr))
        return -1;
    return 0;
}

void SearchThreadManager::_sigusr1Exit(int) { pthread_exit(nullptr); }

#endif
