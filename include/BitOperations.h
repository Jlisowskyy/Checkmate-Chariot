//
// Created by Jlisowskyy on 12/29/23.
//

#ifndef BITOPERATIONS_H
#define BITOPERATIONS_H

#include <bit>
#include <cinttypes>
#include <cstdlib>

/*
 *  This header collects some functions used to manipulate bits in a 64-bit unsigned integers.
 *  The functions are constexpr, so they can be used at compile time.
 *  Most of them depends on c++20 <bit> header, so they allow to perform operations on bits in a very efficient way.
 *  They should also be platform independent with respect to c++20 implementation.
 *  To ensure same style around the project only those functions should be used to manipulate bits.
 *
 * */

/* This values should be used inside every shifting operation to ensure correct types under the operation */
static constexpr uint64_t MinMsbPossible = static_cast<uint64_t>(1);
static constexpr uint64_t MaxMsbPossible = MinMsbPossible << 63;

/* Function efficiently computes MsbPos */
constexpr int ExtractMsbPos(const uint64_t x) { return std::countl_zero(x); }

/* Nice way to reverse from MsbPos to LsbPos and other way around */
constexpr int ConvertToReversedPos(const int x)
{
    return x ^ 63; // equals to 63 - x;
}

/* Simply Runs 'ExtractMsbPos' and applies 'ConvertToReversedPos' on it */
constexpr int ExtractMsbReversedPos(const uint64_t x) { return ConvertToReversedPos(ExtractMsbPos(x)); }

/* Function efficiently computes LsbPos */
constexpr int ExtractLsbPos(const uint64_t x) { return std::countr_zero(x); }

/* Simply Runs 'ExtractMsbPos' and applies 'ConvertToReversedPos' on it */
constexpr int ExtractLsbReversedPos(const uint64_t x) { return ConvertToReversedPos(ExtractLsbPos(x)); }

/* Function does nothing, simply returns given value */
constexpr int NoOp(const int m) { return m; }

/* Functions returns BitMap with MsbPos as 1 */
constexpr uint64_t ExtractMsbBitBuiltin(const uint64_t x) { return MaxMsbPossible >> ExtractMsbPos(x); }

/* Functions returns BitMap with LsbPos as 1 */
constexpr uint64_t ExtractLsbBitBuiltin(const uint64_t x) { return MinMsbPossible << ExtractLsbReversedPos(x); }

/* Functions returns BitMap with LsbPos as 1 */
constexpr uint64_t ExtractLsbOwn1(const uint64_t x) { return x & -x; }

/* Functions returns BitMap with MsbPos as 1, with additional check whether given argument is 0 */
constexpr uint64_t ExtractMsbBit(const uint64_t x) { return x == 0 ? 0 : ExtractMsbBitBuiltin(x); }

/* Functions returns BitMap with LsbPos as 1 */
constexpr uint64_t ExtractLsbBit(const uint64_t x) { return ExtractLsbOwn1(x); }

/* Function simply returns a map with remove all bits that are present on b*/
constexpr uint64_t ClearAFromIntersectingBits(const uint64_t a, const uint64_t b) { return a ^ (a & b); }

/* Function simply counts ones on the given BitMap*/
constexpr int CountOnesInBoard(const uint64_t bitMap) { return std::popcount(bitMap); }

/*          IMPORTANT NOTES:
 *  Function assumes that containerPos is already set to 1
 *  and container[0] is 0 value, which will induce all others.
 *  Use GenerateBitPermutation() wrapper instead.
 */

template <class IndexableT>
constexpr void
GenerateBitPermutationsRecursion(const uint64_t number, const int bitPos, IndexableT &container, size_t &containerPos)
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

template <class IndexableT> constexpr size_t GenerateBitPermutations(const uint64_t number, IndexableT &container)
{
    container[0] = 0;
    size_t index = 1;

    GenerateBitPermutationsRecursion(number, 63, container, index);
    return index;
}

#endif // BITOPERATIONS_H
