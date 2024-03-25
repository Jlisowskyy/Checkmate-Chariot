//
// Created by Jlisowskyy on 12/29/23.
//

#ifndef BITOPERATIONS_H
#define BITOPERATIONS_H

#include <cinttypes>
#include <cstdlib>

static constexpr uint64_t maxMsbPossible = 1LLU << 63;
static constexpr uint64_t minMsbPossible = 1LLU;

constexpr int ExtractMsbPos(const uint64_t x) { return __builtin_clzl(x); }

constexpr int ConvertToReversedPos(const int x)
{
    return x ^ 63;  // equals to 63 - x;
}

constexpr int ExtractMsbReversedPos(const uint64_t x) { return ConvertToReversedPos(ExtractMsbPos(x)); }

constexpr int ExtractLsbReversedPos(const uint64_t x) { return __builtin_ctzl(x); }

constexpr int ExtractLsbPos(const uint64_t x) { return ConvertToReversedPos(ExtractLsbReversedPos(x)); }

constexpr int NoOp(const int m) { return m; }

constexpr uint64_t ExtractMsbBitBuiltin(const uint64_t x) { return maxMsbPossible >> ExtractMsbPos(x); }

constexpr uint64_t ExtractLsbBitBuiltin(const uint64_t x) { return minMsbPossible << ExtractLsbReversedPos(x); }

constexpr uint64_t ExtractLsbOwn1(const uint64_t x) { return x & -x; }

constexpr uint64_t ExtractMsbBit(const uint64_t x) { return x == 0 ? 0 : ExtractMsbBitBuiltin(x); }

constexpr uint64_t ExtractLsbBit(const uint64_t x) { return ExtractLsbOwn1(x); }

constexpr uint64_t ClearAFromIntersectingBits(const uint64_t a, const uint64_t b) { return a ^ (a & b); }

constexpr int CountOnesInBoard(const uint64_t num) { return __builtin_popcountll(num); }

constexpr __uint128_t operator""_uint128_t(const char* x)
{
    __uint128_t y = 0;
    ssize_t literalSize{};

    for (ssize_t i = 0; x[i] != '\0'; ++i)
        literalSize = i;

    __uint128_t pow = 1;
    for (ssize_t i = literalSize; i > -1; --i)
    {
        const __uint128_t temp = (x[i] - '0') * pow;
        pow *= 10ull;

        y += temp;
    }

    return y;
}

/*          IMPORTANT NOTES:
 *  Function assumes that containerPos is already set to 1
 *  and container[0] is 0 value, which will induce all others.
 *  Use GenerateBitPermutation() wrapper instead.
 */

template <class IndexableT>
constexpr void GenerateBitPermutationsRecursion(const uint64_t number, const int bitPos, IndexableT& container,
                                                size_t& containerPos)
{
    if (bitPos == -1 || number == 0)
        return;
    uint64_t nextBit{};

    for (int i = bitPos; i >= 0; --i)
        if (const uint64_t bitMap = 1LLU << i; (bitMap & number) != 0)
        {
            GenerateBitPermutationsRecursion(number ^ bitMap, i - 1, container, containerPos);
            nextBit = bitMap;
            break;
        }

    const size_t rangeEnd = containerPos;
    for (size_t i = 0; i < rangeEnd; ++i)
    {
        container[rangeEnd + i] = container[i] | nextBit;
    }

    containerPos *= 2;
}

template <class IndexableT>
constexpr size_t GenerateBitPermutations(const uint64_t number, IndexableT& container)
{
    container[0] = 0;
    size_t index = 1;

    GenerateBitPermutationsRecursion(number, 63, container, index);
    return index;
}

#endif  // BITOPERATIONS_H
