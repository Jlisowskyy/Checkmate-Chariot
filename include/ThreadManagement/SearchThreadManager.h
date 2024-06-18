//
// Created by Jlisowskyy on 3/7/24.
//

#ifndef SEARCHTHREADMANAGER_H
#define SEARCHTHREADMANAGER_H

#include <map>
#include <semaphore>
#include <string>
#include <thread>

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"
#include "Stack.h"

class SearchThreadManager
{
    // ------------------------------
    // Class inner types
    // ------------------------------

    struct _searchArgs_t
    {
        const Board *bd;
        int depth;
    };

    public:
    using StackType = Stack<Move, DEFAULT_STACK_SIZE>;

    // ------------------------------
    // Class creation
    // ------------------------------

    SearchThreadManager();

    ~SearchThreadManager();

    SearchThreadManager(const SearchThreadManager &) = delete;
    SearchThreadManager(SearchThreadManager &&)      = delete;

    SearchThreadManager &operator=(const SearchThreadManager &) = delete;
    SearchThreadManager &operator=(SearchThreadManager &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] StackType &GetDefaultStack() { return _stacks[0]; }

    bool Go(const Board &bd, const GoInfo &info);

    /* This function is not thread safe! Use it when there is no time left on the clock to start a thread */
    static void GoWoutThread(const Board &bd, const GoInfo &info);

    bool GoInfinite(const Board &bd);

    void Stop() const;

    [[nodiscard]] bool IsSearchOn() const { return _isSearchOn; }

    [[nodiscard]] bool IsPonderOn() const { return _isPonderOn; }

    void DisablePonder() { _isPonderOn = false; }

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    static void _passiveThreadSearchJob(
        Stack<Move, DEFAULT_STACK_SIZE> *s, _searchArgs_t *args, bool *guard, const bool *shouldStop,
        std::binary_semaphore *taskSem, std::binary_semaphore *bootup
    );

    // ------------------------------
    // Class fields
    // ------------------------------

    bool _isSearchOn{false};
    bool _isPonderOn{false};

    // Passive thread components
    bool _shouldStop{false};
    std::binary_semaphore _searchSem{0};
    std::binary_semaphore _bootupSem{0};
    _searchArgs_t _searchArgs{};

    // TODO: Implement logical thread detection

    static constexpr size_t MaxSearchThreads   = 20;
    static constexpr size_t MaxManagingThreads = 1;
    static constexpr size_t MaxThreadCount     = MaxSearchThreads + MaxManagingThreads;

    StackType _stacks[MaxThreadCount]{};
    std::thread *_threads[MaxThreadCount]{};

    static constexpr size_t MainSearchThreadInd = 0;
};

#endif // SEARCHTHREADMANAGER_H
