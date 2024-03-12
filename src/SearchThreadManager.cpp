//
// Created by Jlisowskyy on 3/7/24.
//

#include "../include/ThreadManagement/SearchThreadManager.h"

#include <cstring>

#ifdef __unix__

#include <unistd.h>
#include <csignal>

#endif  // __unix__

SearchThreadManager::~SearchThreadManager()
{
    for (const auto thread : _threads)
        delete thread;
}

bool SearchThreadManager::goInfinite(const Board& bd)
{
    if (_isSearchOn == true)
        return false;

    _threads[0] = new std::thread(_threadSearchJob, &bd, &_stacks[0], &_seachResult, INT_MAX);
    _isSearchOn = true;

    return true;
}

std::string SearchThreadManager::stop()
{
    if (_isSearchOn == false)
        return "";

    _cancelThread(0);
    _isSearchOn = false;
    return _seachResult.GetLongAlgebraicNotation();
}

std::string SearchThreadManager::goMoveTime(const Board& bd, long long msecs)
{
    if (_isSearchOn == true)
        return "";

    goInfinite(bd);
    std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
    return stop();
}

std::string SearchThreadManager::goDepth(const Board& bd, int depth)
{
    if (_isSearchOn == true)
        return "";

    _threads[0] = new std::thread(_threadSearchJob, &bd, &_stacks[0], &_seachResult, depth);
    _threads[0]->join();

    delete _threads[0];
    _threads[0] = nullptr;
    _stacks[0].Clear();

    return _seachResult.GetLongAlgebraicNotation();
}

void SearchThreadManager::_threadSearchJob(const Board* bd, stack<Move, DefaultStackSize>* s, Move* output,
                                           const int depth)
{
#ifdef __unix__
    if (_sethandler(_sigusr1_exit, SIGUSR1))
        exit(EXIT_FAILURE);
#endif  // __unix__

    BestMoveSearch searcher{*bd, *s};
    searcher.IterativeDeepening(BoardEvaluator::DefaultFullEvalFunction, output, depth);
}

void SearchThreadManager::_cancelThread(const size_t threadInd)
{
#ifdef __unix__
    const pthread_t tid = _threads[threadInd]->native_handle();
    pthread_kill(tid, SIGUSR1);
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
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, nullptr))
        return -1;
    return 0;
}

void SearchThreadManager::_sigusr1_exit(int) { pthread_exit(nullptr); }

#endif
