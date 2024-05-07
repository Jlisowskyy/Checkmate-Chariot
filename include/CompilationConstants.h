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

#endif // COMPILATION_CONSTANTS_H
