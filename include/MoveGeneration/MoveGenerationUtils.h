//
// Created by Jlisowskyy on 12/30/23.
//

#ifndef MOVEGENERATION_H
#define MOVEGENERATION_H

#include <array>
#include <bit>

#include "../BitOperations.h"
#include "../EngineUtils.h"
#include "../Interface/Logger.h"

[[nodiscard]] constexpr std::array<uint64_t, Board::BitBoardFields>
GenStaticMoves(const size_t maxMovesCount, const int *movesCords, const int *rowCords)
{
    std::array<uint64_t, Board::BitBoardFields> movesRet{};

    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            const int bInd   = 8 * y + x;
            const int msbInd = ConvertToReversedPos(bInd);

            uint64_t packedMoves = 0;
            for (size_t i = 0; i < maxMovesCount; ++i)
            {
                const int moveYCord   = (bInd + movesCords[i]) / 8;
                const int knightYCord = bInd / 8;

                if (const int moveInd = bInd + movesCords[i];
                    moveInd >= 0 && moveInd < 64 && moveYCord == knightYCord + rowCords[i])
                    packedMoves |= 1LLU << moveInd;
            }

            movesRet[msbInd] = packedMoves;
        }
    }

    return movesRet;
}

template <class NeighborCountingFuncT>
[[nodiscard]] constexpr size_t CalculateTotalOfPossibleHashMapElements(NeighborCountingFuncT func)
{
    size_t sum{};
    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            sum += func(x, y);
        }
    }

    return sum;
}

inline void DisplayMasks(const uint64_t *masks, const char **names, const size_t maskCount)
{
    for (size_t i = 0; i < maskCount; ++i)
    {
        GlobalLogger.LogStream << "Mask name: " << names[i] << '\n';
        DisplayMask(masks[i]);
    }
}

template <class ComparisonMethodT>
[[nodiscard]] constexpr uint64_t GenMask(const int barrier, int boardIndex, const int offset, ComparisonMethodT comp)
{
    uint64_t mask = 0;

    while (comp(boardIndex += offset, barrier)) mask |= (1LLU << boardIndex);

    return mask;
}

[[nodiscard]] constexpr uint64_t GenMask(const int startInd, const int boarderIndex, const int offset)
{
    uint64_t ret = 0;
    for (int i = startInd; i < boarderIndex; i += offset) ret |= (1LLU << i);
    return ret;
}

[[nodiscard]] constexpr size_t MyCeil(const double x)
{
    return (static_cast<double>(static_cast<size_t>(x)) == x) ? static_cast<size_t>(x)
                                                              : static_cast<size_t>(x) + ((x > 0) ? 1 : 0);
}

[[nodiscard]] constexpr size_t GetRoundedSizePow(const size_t x) { return std::countr_zero(std::bit_ceil(x)); }

template <class BoundryCheckFuncT>
constexpr uint64_t
GenSlidingMoves(const uint64_t neighbors, const int bInd, const int offset, BoundryCheckFuncT boundryCheck)
{
    uint64_t ret = 0;
    int actPos   = bInd;

    while (boundryCheck(actPos += offset))
    {
        const uint64_t curMove = 1LLU << actPos;
        ret |= curMove;

        if ((curMove & neighbors) != 0)
            break;
    }

    return ret;
}

template <class MoveGeneratorT, class NeighborGeneratorT, class NeigborStripT, class MapT>
constexpr void
MoveInitializer(MapT &map, MoveGeneratorT mGen, NeighborGeneratorT nGen, NeigborStripT nStrip, const int bInd)
{
    const auto [possibilities, posSize] = nGen(bInd, map.masks);

    for (size_t j = 0; j < posSize; ++j)
    {
        const uint64_t strippedNeighbors = nStrip(possibilities[j], map.masks);
        const uint64_t moves             = mGen(strippedNeighbors, bInd);

        map[possibilities[j]] = moves;
    }
}

#endif // MOVEGENERATION_H
