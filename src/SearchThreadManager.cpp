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

bool SearchThreadManager::goInfinite(const Board &bd, const uint16_t age)
{
    if (_isSearchOn)
        return false;

    _threads[0] = new std::thread(_threadSearchJob, &bd, &_stacks[0], &_seachResult, MaxSearchDepth, age);
    _isSearchOn = true;

    return true;
}

std::string SearchThreadManager::stop()
{
    if (!_isSearchOn)
        return "";

    _cancelThread(0);
    _isSearchOn = false;
    return _seachResult.GetLongAlgebraicNotation();
}

std::string SearchThreadManager::goMoveTime(const Board &bd, const long long msecs, const uint16_t age)
{
    if (_isSearchOn)
        return "";

    goInfinite(bd, age);
    std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
    return stop();
}

std::string SearchThreadManager::goDepth(const Board &bd, int depth, const uint16_t age)
{
    if (_isSearchOn)
        return "";

    _threads[0] =
        new std::thread(_threadSearchJob, &bd, &_stacks[0], &_seachResult, std::min(depth, MaxSearchDepth), age);
    _threads[0]->join();

    delete _threads[0];
    _threads[0] = nullptr;
    _stacks[0].Clear();

    return _seachResult.GetLongAlgebraicNotation();
}

void SearchThreadManager::_threadSearchJob(
    const Board *bd, stack<Move, DefaultStackSize> *s, PackedMove *output, const int depth, const uint16_t age
)
{
#ifdef __unix__
    if (_sethandler(_sigusr1_exit, SIGUSR1))
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

int SearchThreadManager::_sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act = {};
    act.sa_handler       = f;
    if (-1 == sigaction(sigNo, &act, nullptr))
        return -1;
    return 0;
}

void SearchThreadManager::_sigusr1_exit(int) { pthread_exit(nullptr); }

#endif
