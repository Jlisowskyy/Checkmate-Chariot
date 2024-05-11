//
// Created by Jlisowskyy on 3/7/24.
//

#ifndef SEARCHTHREADMANAGER_H
#define SEARCHTHREADMANAGER_H

#include <string>
#include <thread>

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"
#include "../cmakeDefines.h"
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

    bool Go(const Board &bd, uint16_t age, const GoInfo &info);

    bool GoInfinite(const Board &bd, uint16_t age);

    void Stop();

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    static void
    _threadSearchJob(const Board *bd, Stack<Move, DefaultStackSize> *s, bool *guard, uint16_t age, int depth);

    // ------------------------------
    // Class fields
    // ------------------------------

    bool _isSearchOn{false};

    // TODO: Implement logical thread detection
    StackType _stacks[20 + 1]{};
    std::thread *_threads[20 + 1]{};

    static constexpr size_t MainSearchThreadInd = 0;
};

#endif // SEARCHTHREADMANAGER_H
