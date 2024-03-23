//
// Created by Jlisowskyy on 1/1/24.
//

#ifndef ROOKMAPGENERATOR_H
#define ROOKMAPGENERATOR_H

#include <array>

#include "../BitOperations.h"
#include "MoveGeneration.h"

class RookMapGenerator
{
   public:
    static constexpr size_t MaxRookPossibleNeighborsWoutOverlap = 144;
    static constexpr size_t MaxRookPossibleNeighborsWithOverlap = 4096;

    static constexpr int NorthOffset = 8;
    static constexpr int SouthOffset = -8;
    static constexpr int WestOffset = -1;
    static constexpr int EastOffset = 1;

   private:
    static constexpr size_t DirectedMaskCount = 4;

   public:
    using MasksT = std::array<uint64_t, DirectedMaskCount>;

    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    RookMapGenerator() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] constexpr static MasksT InitMasks(int bInd);

    [[nodiscard]] constexpr static uint64_t GenMoves(uint64_t neighborsWoutOverlap, int bInd);

    [[nodiscard]] constexpr static std::tuple<std::array<uint64_t, MaxRookPossibleNeighborsWoutOverlap>, size_t>
    GenPossibleNeighborsWoutOverlap(int bInd, const MasksT& masks);

    [[nodiscard]] constexpr static std::tuple<std::array<uint64_t, MaxRookPossibleNeighborsWithOverlap>, size_t>
    GenPossibleNeighborsWithOverlap(const MasksT& masks);

    [[nodiscard]] static constexpr size_t PossibleNeighborWoutOverlapCountOnField(int x, int y);

    static constexpr uint64_t StripBlockingNeighbors(uint64_t fullBoard, const MasksT& masks);

    // ------------------------------
    // Class inner types
    // ------------------------------

    enum maskInd
    {
        wMask,
        eMask,
        nMask,
        sMask,
    };
};

constexpr RookMapGenerator::MasksT RookMapGenerator::InitMasks(const int bInd)
{
    constexpr int SouthBarrier = 7;
    constexpr int NorthBarrier = 56;

    std::array<uint64_t, DirectedMaskCount> ret{};

    // Mask generation.
    const int westBarrier = (bInd / 8) * 8;
    const int eastBarrier = westBarrier + 7;

    // South vertical line
    ret[sMask] = GenMask(SouthBarrier, bInd, SouthOffset, std::greater{});

    // West Horizontal line
    ret[wMask] = GenMask(westBarrier, bInd, WestOffset, std::greater{});

    // East horizontal line
    ret[eMask] = GenMask(eastBarrier, bInd, EastOffset, std::less{});

    // North vertical line
    ret[nMask] = GenMask(NorthBarrier, bInd, NorthOffset, std::less{});

    return ret;
}

constexpr uint64_t RookMapGenerator::GenMoves(const uint64_t neighborsWoutOverlap, const int bInd)
{
    constexpr int northBarrier = 64;
    constexpr int southBarrier = -1;
    const int westBarrier = (bInd / 8) * 8 - 1;
    const int eastBarrier = westBarrier + 9;

    uint64_t moves = 0;

    // North lines moves
    moves |=
            GenSlidingMoves(neighborsWoutOverlap, bInd, NorthOffset, [&](const int x) { return x < northBarrier; });

    // South lines moves
    moves |=
            GenSlidingMoves(neighborsWoutOverlap, bInd, SouthOffset, [&](const int x) { return x > southBarrier; });

    // East line moves
    moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, EastOffset, [&](const int x) { return x < eastBarrier; });

    // West line moves
    moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, WestOffset, [&](const int x) { return x > westBarrier; });

    return moves;
}

constexpr std::tuple<std::array<uint64_t, RookMapGenerator::MaxRookPossibleNeighborsWoutOverlap>, size_t>
RookMapGenerator::GenPossibleNeighborsWoutOverlap(const int bInd, const MasksT& masks)
{
    std::array<uint64_t, MaxRookPossibleNeighborsWoutOverlap> ret{};
    size_t usedFields = 0;

    const int westBarrier = ((bInd >> 3) << 3) - 1;
    const int eastBarrier = westBarrier + 9;
    constexpr int northBarrier = 64;
    constexpr int southBarrier = -1;

    const uint64_t bPos = 1LLU << bInd;
    for (int westCord = bInd; westCord > westBarrier; westCord += WestOffset)
    {
        const uint64_t westPos = minMsbPossible << westCord;
        if (westPos != bPos && (masks[wMask] & westPos) == 0)
            continue;

        for (int eastCord = bInd; eastCord < eastBarrier; eastCord += EastOffset)
        {
            const uint64_t eastPos = minMsbPossible << eastCord;
            if (eastPos != bPos && (masks[eMask] & eastPos) == 0)
                continue;

            for (int northCord = bInd; northCord < northBarrier; northCord += NorthOffset)
            {
                const uint64_t northPos = minMsbPossible << northCord;
                if (northPos != bPos && (masks[nMask] & northPos) == 0)
                    continue;

                for (int southCord = bInd; southCord > southBarrier; southCord += SouthOffset)
                {
                    const uint64_t southPos = minMsbPossible << southCord;
                    if (southPos != bPos && (masks[sMask] & southPos) == 0)
                        continue;

                    const uint64_t neighbor = (southPos | northPos | eastPos | westPos) & ~bPos;
                    ret[usedFields++] = neighbor;
                }
            }
        }
    }

    return {ret, usedFields};
}

constexpr std::tuple<std::array<uint64_t, RookMapGenerator::MaxRookPossibleNeighborsWithOverlap>, size_t>
RookMapGenerator::GenPossibleNeighborsWithOverlap(const MasksT& masks)
{
    std::array<uint64_t, MaxRookPossibleNeighborsWithOverlap> ret{};
    const uint64_t fullMask = masks[nMask] | masks[sMask] | masks[eMask] | masks[wMask];

    size_t usedFields = GenerateBitPermutations(fullMask, ret);

    return {ret, usedFields};
}

constexpr size_t RookMapGenerator::PossibleNeighborWoutOverlapCountOnField(const int x, const int y)
{
    const int westCount = std::max(1, x);
    const int southCount = std::max(1, y);
    const int northCount = std::max(1, 7 - y);
    const int eastCount = std::max(1, 7 - x);

    return westCount * eastCount * southCount * northCount;
}

constexpr uint64_t RookMapGenerator::StripBlockingNeighbors(const uint64_t fullBoard, const MasksT& masks)
{
    const uint64_t northPart = ExtractLsbBit(fullBoard & masks[nMask]);
    const uint64_t southPart = ExtractMsbBit(fullBoard & masks[sMask]);
    const uint64_t westPart = ExtractMsbBit(fullBoard & masks[wMask]);
    const uint64_t eastPart = ExtractLsbBit(fullBoard & masks[eMask]);
    return northPart | southPart | westPart | eastPart;
}

#endif  // ROOKMAPGENERATOR_H
