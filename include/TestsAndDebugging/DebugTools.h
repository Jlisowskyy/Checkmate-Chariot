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

/* Stack trace debug function */
inline void TRACE_HANDLER(int sig) {
    static constexpr size_t STACK_SIZE = 256;
    void *array[STACK_SIZE];

    // get void*'s for all entries on the stack
    const int size = backtrace(array, STACK_SIZE);

    // print out all the frames to stderr
    fprintf(stderr, "Error received signal -> %s\n", strsignal(sig));
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}


/* Prints information about actual stack usage */
inline void TRACE_STACK_USAGE()
{
    struct rlimit limit{};
    getrlimit (RLIMIT_STACK, &limit);

    const rlim_t unusedBytes = limit.rlim_max - limit.rlim_cur;
    const double availableSpace = (double)unusedBytes / (double)limit.rlim_max;
    const double usage = (double)limit.rlim_cur / (double)limit.rlim_max;

    GlobalLogger.LogStream << std::format("[ STACK INFO ] Queried following data about the stack:\n"
                                          "\tUnused bytes: {}\n"
                                          "\tUsage: {}\n"
                                          "\tAvailable space: {}\n", unusedBytes, usage, availableSpace);
}

#endif // __unix__

#endif // CHECKMATE_CHARIOT_DEBUGTOOLS_H
