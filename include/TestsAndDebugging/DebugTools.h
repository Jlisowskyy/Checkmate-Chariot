//
// Created by Jlisowskyy on 5/20/24.
//

#ifndef CHECKMATE_CHARIOT_DEBUGTOOLS_H
#define CHECKMATE_CHARIOT_DEBUGTOOLS_H

#include <queue>
#include <tuple>

#include "../CompilationConstants.h"
#include "../Interface/Logger.h"
#include "../MoveGeneration/Move.h"

/*
 * struct defines statistics gathered during execution about aspiration window flow
 * */

class AspWinStat
{
    enum class FailType
    {
        FailLow,
        FailHigh,
        FinalBoundaries
    };

    std::queue<std::tuple<FailType, int, int, int>> _fails{};

    public:
    void RetryFailLow(int alpha, int beta, int eval) { _fails.emplace(FailType::FailLow, alpha, beta, eval); }
    void RetryFailHigh(int alpha, int beta, int eval) { _fails.emplace(FailType::FailHigh, alpha, beta, eval); }
    void RecordFinalBoundaries(int alpha, int beta, int eval)
    {
        _fails.emplace(FailType::FinalBoundaries, alpha, beta, eval);
    }

    void DisplayAndClean();
};

bool IsDrawDebug(const Board &bd);

Move GetMoveDebug(const Board &bd, const std::string &str);

#ifdef __unix__

#include <cstdio>
#include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/resource.h>
#include <pthread.h>

inline void DISPLAY_BACKTRACE()
{
    static constexpr size_t STACK_SIZE = 256;
    void *array[STACK_SIZE];

    // get void*'s for all entries on the stack
    const int size = backtrace(array, STACK_SIZE);

    // print out all the frames to stderr
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    fflush(stderr);
    fflush(stdout);
}

/* Stack trace debug function */
inline void TRACE_HANDLER(int sig) {
    fprintf(stderr, "Error received signal -> %s\n", strsignal(sig));
    DISPLAY_BACKTRACE();
    exit(EXIT_FAILURE);
}

inline size_t GetCurrStackPtr() {
    // This function returns the address of a local variable,
    // which is used as an approximation of the current stack pointer.
    int local;
    return reinterpret_cast<size_t>(&local);
}

extern __thread size_t StackStartAddress;
inline const size_t StackSize = [](){
    struct rlimit limits{};
    getrlimit(RLIMIT_STACK, &limits);

    return limits.rlim_cur;
}();

/* Prints information about actual stack usage */
inline void TRACE_STACK_USAGE()
{
    // get approx of current stack addr
    const size_t currentSP = GetCurrStackPtr();
    const size_t startSP = StackStartAddress;
    const size_t usedStack = startSP - currentSP;
    const size_t availableStack = StackSize - usedStack;

    const double availableSpace = (double)availableStack / (double)StackSize;

    GlobalLogger.LogStream << std::format("[ STACK INFO ] Queried following data about the stack:"
                                          " Unused bytes: {}"
                                          " Used bytes: {}"
                                          " Available space: {}", availableStack, usedStack, availableSpace) << std::endl;

    if (availableStack < 0.1)
        GlobalLogger.LogStream << "[ WARNING ] Stack available space dropped below 0.1!" << std::endl;
}


#endif // __unix__

#endif // CHECKMATE_CHARIOT_DEBUGTOOLS_H
