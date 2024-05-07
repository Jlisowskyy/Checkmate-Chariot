//
// Created by Jlisowskyy on 3/7/24.
//

#ifndef SEARCHTHREADMANAGER_H
#define SEARCHTHREADMANAGER_H

#include <string>
#include <thread>

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"
#include "Stack.h"

struct SearchThreadManager
{
    using StackType = Stack<Move, DefaultStackSize>;
    // ------------------------------
    // Class creation
    // ------------------------------

    SearchThreadManager() = default;
    ~SearchThreadManager();

    SearchThreadManager(const SearchThreadManager &) = delete;
    SearchThreadManager(SearchThreadManager &&)      = delete;

    SearchThreadManager &operator=(const SearchThreadManager &) = delete;
    SearchThreadManager &operator=(SearchThreadManager &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] StackType &GetDefaultStack() { return _stacks[0]; }

    bool GoInfinite(const Board &bd, uint16_t age);

    std::string GoMoveTime(const Board &bd, long long msecs, uint16_t age);

    std::string GoDepth(const Board &bd, int depth, uint16_t age);

    std::string Stop();

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    static void
    _threadSearchJob(const Board *bd, Stack<Move, DefaultStackSize> *s, PackedMove *output, int depth, uint16_t age);

    void _cancelThread(size_t threadInd);

#ifdef __unix__

    static int _setHandler(void (*f)(int), int sigNo);
    static void _sigusr1Exit(int);

#endif

    // ------------------------------
    // Class fields
    // ------------------------------

    bool _isSearchOn{false};
    PackedMove _searchResult{};

    StackType _stacks[20 + 1]{};
    std::thread *_threads[20 + 1]{};
};

#endif // SEARCHTHREADMANAGER_H
