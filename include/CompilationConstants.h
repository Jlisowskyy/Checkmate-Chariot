//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef COMPILATION_CONSTANTS_H
#define COMPILATION_CONSTANTS_H

#include <cinttypes>

using lli                  = long long int;
static constexpr size_t MB = 1024 * 1024;

// global defined Stack capacity used to store generated moves per thread
static constexpr size_t DefaultStackSize = 32 * MB / sizeof(uint64_t);

/* Defines maximal depth of search allowed across the procjet */
static constexpr int MaxSearchDepth = 128;

static constexpr uint64_t MsesInNsec = 1000 * 1000;

// ------------------------------
// Platform specific defines for force inline attribute

#ifndef PREVENT_FORCE_INLINE

#ifdef _MSC_VER

// MSVC specific force inline attribute
#define INLINE __forceinline

#else

// GNU / CLANG force inline specific attribute
#define INLINE __attribute__((always_inline))

#endif

#endif // PREVENT_FORCE_INLINE

// ------------------------------
// ------------------------------
// Platform specific defines for path separator

#ifdef __WIN32__

#define SLASH '\\'

#else

#define SLASH '/'

#endif

// ------------------------------

// ssize_t is defined by POSIX. Such define allows to use it on posix+windows

using signed_size_t = std::make_signed<size_t>::type;

#endif // COMPILATION_CONSTANTS_H
