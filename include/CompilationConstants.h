//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef COMPILATION_CONSTANTS_H
#define COMPILATION_CONSTANTS_H

#include <cinttypes>

using lli = long long int;

// global defined Stack capacity used to store generated moves per thread
static constexpr size_t MB               = 1024 * 1024;
static constexpr size_t DefaultStackSize = 32 * MB / sizeof(uint64_t);
static constexpr int MaxSearchDepth      = 128;

static constexpr uint64_t msescInNsec = 1000 * 1000; // in nsecs

#ifdef _MSC_VER

// MSVC specific force inline attribute
#define INLINE __forceinline

#else

// GNU / CLANG force inline specific attribute
#define INLINE __attribute__((always_inline))

#endif

// ssize_t is POSIX. This is how we use it on posix+windows
using signed_size_t = std::make_signed<size_t>::type;

#endif // COMPILATION_CONSTANTS_H
