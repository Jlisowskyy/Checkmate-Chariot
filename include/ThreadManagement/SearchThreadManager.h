//
// Created by Jlisowskyy on 3/7/24.
//

#ifndef SEARCHTHREADMANAGER_H
#define SEARCHTHREADMANAGER_H

#include <string>
#include <thread>
#include <map>

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"
#include "Stack.h"

struct SearchThreadManager
{
    using StackType = Stack<Move, DEFAULT_STACK_SIZE>;
    using RepMap = std::map<uint64_t, int>;
    // ------------------------------
    // Class creation
    // ------------------------------

    SearchThreadManager() = default;

    // TODO: Temporary solution, should be expanded with threading model
    ~SearchThreadManager();

    SearchThreadManager(const SearchThreadManager &) = delete;
    SearchThreadManager(SearchThreadManager &&)      = delete;

    SearchThreadManager &operator=(const SearchThreadManager &) = delete;
    SearchThreadManager &operator=(SearchThreadManager &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] StackType &GetDefaultStack() { return _stacks[0]; }

    bool Go(const Board &bd, const RepMap& rMap, uint16_t age, const GoInfo &info);

    /* Function is not thread safe! Is when there is no time to start up threads! */
    static void GoWoutThread(const Board &bd, const RepMap& rMap, uint16_t age, const GoInfo &info);

    bool GoInfinite(const Board &bd, const RepMap& rMap, uint16_t age);

    void Stop();

    // TODO: Temporary solution, should be expanded with threading model
    void Consolidate();

    [[nodiscard]] bool IsSearchOn() const { return _isSearchOn; }

    [[nodiscard]] bool IsPonderOn() const { return _isPonderOn; }

    void DisablePonder() { _isPonderOn = false; }

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    static void
    _threadSearchJob(const Board *bd, const RepMap* rMap, Stack<Move, DEFAULT_STACK_SIZE> *s, bool *guard, uint16_t age, int depth);

    // ------------------------------
    // Class fields
    // ------------------------------

    bool _isSearchOn{false};
    bool _isPonderOn{false};

    // TODO: Implement logical thread detection

    static constexpr size_t MaxSearchThreads   = 20;
    static constexpr size_t MaxManagingThreads = 1;
    static constexpr size_t MaxThreadCount     = MaxSearchThreads + MaxManagingThreads;

    StackType _stacks[MaxThreadCount]{};
    std::thread *_threads[MaxThreadCount]{};

    static constexpr size_t MainSearchThreadInd = 0;
};

#endif // SEARCHTHREADMANAGER_H
