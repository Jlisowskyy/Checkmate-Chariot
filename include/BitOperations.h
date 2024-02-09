//
// Created by Jlisowskyy on 12/29/23.
//

#ifndef BITOPERATIONS_H
#define BITOPERATIONS_H

#include <cinttypes>
#include <cstdlib>

static constexpr uint64_t maxMsbPossible = 1LLU<<63;
static constexpr uint64_t minMsbPossible = 1LLU;

constexpr int ExtractMsbPos(const uint64_t x) {
    return __builtin_clzl(x);
}

constexpr int ConvertToReversedPos(const int x) {
    return x ^ 63; // equals to 63 - x;
}

constexpr int ExtractMsbReversedPos(const uint64_t x) {
    return ConvertToReversedPos(ExtractMsbPos(x));
}

constexpr int ExtractLsbReversedPos(const uint64_t x) {
    return __builtin_ctzl(x);
}

constexpr int ExtractLsbPos(const uint64_t x) {
    return ConvertToReversedPos(ExtractLsbReversedPos(x));
}

constexpr uint64_t ExtractMsbBitBuiltin(const uint64_t x) {
    return maxMsbPossible >> ExtractMsbPos(x);
}

constexpr uint64_t ExtractLsbBitBuiltin(const uint64_t x) {
    return minMsbPossible << ExtractLsbReversedPos(x);
}

constexpr uint64_t ExtractLsbOwn1(const uint64_t x) {
    return x & -x;
}

constexpr uint64_t ExtractMsbBit(const uint64_t x) {
    return ExtractMsbBitBuiltin(x);
}

constexpr uint64_t ExtractLsbBit(const uint64_t x) {
    return ExtractLsbOwn1(x);
}

constexpr uint64_t ClearAFromIntersectingBits(const uint64_t a, const uint64_t b) {
    return a ^ (a & b);
}

constexpr __uint128_t operator""_uint128_t(const char* x)
{
    __uint128_t y = 0;
    ssize_t literalSize{};

    for (ssize_t i = 0; x[i]!='\0'; ++i) literalSize = i;

    __uint128_t pow = 1;
    for (ssize_t i = literalSize; i > -1; --i)
    {
        const __uint128_t temp =  (x[i] - '0') * pow;
        pow *= 10ull;

        y += temp;
    }

    return y;
}

#endif //BITOPERATIONS_H
