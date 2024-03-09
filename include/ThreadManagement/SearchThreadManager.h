//
// Created by Jlisowskyy on 3/7/24.
//

#ifndef SEARCHTHREADMANAGER_H
#define SEARCHTHREADMANAGER_H

#include <thread>
#include <string>

#include "stack.h"
#include "../MoveGeneration/Move.h"
#include "../Evaluation/BoardEvaluator.h"
#include "../Search/BestMoveSearch.h"

class SearchThreadManager
{
    // ------------------------------
    // Class creation
    // ------------------------------
public:

    SearchThreadManager() = default;
    ~SearchThreadManager();

    SearchThreadManager(const SearchThreadManager&) = delete;
    SearchThreadManager(SearchThreadManager&&) = delete;

    SearchThreadManager& operator=(const SearchThreadManager&) = delete;
    SearchThreadManager& operator=(SearchThreadManager&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] stack<Move, DefaultStackSize>& GetDefaultStack()
    {
        return _stacks[0];
    }

    bool goInfinite(const Board& bd);

    std::string stop();

    std::string goMoveTime(const Board& bd, long long msecs);

    std::string goDepth(const Board& bd, int depth);

    // ------------------------------
    // Private class methods
    // ------------------------------
private:

    static void _threadSearchJob(const Board* bd, stack<Move, DefaultStackSize>* s, Move* output, int depth);

    void _cancelThread(size_t threadInd);

#ifdef __unix__

    static int _sethandler(void (*f)(int), int sigNo);
    static void _sigusr1_exit(int);

#endif

    // ------------------------------
    // Class fields
    // ------------------------------

    bool _isSearchOn { false };
    Move _seachResult{};

    stack<Move, DefaultStackSize> _stacks[20 + 1]{};
    std::thread* _threads[20 + 1]{};
};

#endif //SEARCHTHREADMANAGER_H