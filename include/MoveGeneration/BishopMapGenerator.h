//
// Created by Jlisowskyy on 1/1/24.
//

#ifndef BISHOPMAPGENERATOR_H
#define BISHOPMAPGENERATOR_H

#include "MoveGeneration.h"
#include "../BitOperations.h"
#include "../EngineTypeDefs.h"

class BishopMapGenerator
{
public:
    static constexpr size_t MaxPossibleNeighborsWoutOverlap = 108;
    static constexpr size_t MaxPossibleNeighborsWithOverlap = 512;

    static constexpr int NWOffset = 7;
    static constexpr int NEOffset = 9;
    static constexpr int SWOffset = -9;
    static constexpr int SEOffset = -7;

private:
    static constexpr size_t DirectedMaskCount = 4;

public:
    using MasksT = std::array<uint64_t, DirectedMaskCount>;

    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    BishopMapGenerator() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr std::tuple<std::array<uint64_t, MaxPossibleNeighborsWoutOverlap>, size_t>
    GenPossibleNeighborsWoutOverlap(const int bInd, const MasksT&masks)
    {
        std::array<uint64_t, MaxPossibleNeighborsWoutOverlap> ret{};
        size_t usedFields = 0;

        const int x = bInd % 8;
        const int y = bInd / 8;

        const int NWBorder = bInd + NWOffset * std::min(x, 7 - y);
        const int NEBorder = bInd + NEOffset * std::min(7 - x, 7 - y);
        const int SWBorder = bInd + SWOffset * std::min(x, y);
        const int SEBorder = bInd + SEOffset * std::min(7 - x, y);

        const uint64_t bPos = 1LLU << bInd;
        for (int nw = bInd; nw <= NWBorder; nw += NWOffset)
        {
            const uint64_t nwPos = minMsbPossible << nw;
            if (nwPos != bPos && (masks[nwMask] & nwPos) == 0)
                continue;

            for (int ne = bInd; ne <= NEBorder; ne += NEOffset)
            {
                const uint64_t nePos = minMsbPossible << ne;
                if (nePos != bPos && (masks[neMask] & nePos) == 0)
                    continue;

                for (int sw = bInd; sw >= SWBorder; sw += SWOffset)
                {
                    const uint64_t swPos = minMsbPossible << sw;
                    if (swPos != bPos && (masks[swMask] & swPos) == 0)
                        continue;

                    for (int se = bInd; se >= SEBorder; se += SEOffset)
                    {
                        const uint64_t sePos = minMsbPossible << se;
                        if (sePos != bPos && (masks[seMask] & sePos) == 0)
                            continue;

                        const uint64_t neighbor = (nwPos | nePos | swPos | sePos) & ~bPos;
                        ret[usedFields++] = neighbor;
                    }
                }
            }
        }

        return {ret, usedFields};
    }

    [[nodiscard]] static constexpr std::tuple<std::array<uint64_t, MaxPossibleNeighborsWithOverlap>, size_t>
    GenPossibleNeighborsWithOverlap(const MasksT&masks)
    {
        std::array<uint64_t, MaxPossibleNeighborsWithOverlap> ret{};
        const uint64_t fullMask = masks[neMask] | masks[nwMask] | masks[seMask] | masks[swMask];

        size_t usedFields = GenerateBitPermutations(fullMask, ret);

        return {ret, usedFields};
    }


    [[nodiscard]] static constexpr uint64_t GenMoves(const uint64_t neighborsWoutOverlap, const int bInd)
    {
        const int y = bInd / 8;
        const int x = bInd % 8;

        const int NWBorder = bInd + 7 * std::min(x, 7 - y);
        const int NEBorder = bInd + 9 * std::min(7 - x, 7 - y);
        const int SWBorder = bInd - 9 * std::min(x, y);
        const int SEBorder = bInd - 7 * std::min(7 - x, y);

        uint64_t moves = 0;

        // NW direction moves
        moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, NWOffset,
                                 [&](const int b) { return b <= NWBorder; }
        );

        // NE direction moves
        moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, NEOffset,
                                 [&](const int b) { return b <= NEBorder; }
        );

        // SW direction moves
        moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, SWOffset,
                                 [&](const int b) { return b >= SWBorder; }
        );

        // SE direction moves
        moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, SEOffset,
                                 [&](const int b) { return b >= SEBorder; }
        );

        return moves;
    }

    [[nodiscard]] static constexpr size_t PossibleNeighborWoutOverlapCountOnField(const int x, const int y)
    {
        const size_t nwCount = std::max(1, std::min(x, 7 - y));
        const size_t neCount = std::max(1, std::min(7 - x, 7 - y));
        const size_t swCount = std::max(1, std::min(x, y));
        const size_t seCount = std::max(1, std::min(7 - x, y));

        return nwCount * neCount * swCount * seCount;
    }


    [[nodiscard]] static constexpr MasksT InitMasks(const int bInd)
    {
        std::array<uint64_t, DirectedMaskCount> ret{};
        const int x = bInd % 8;
        const int y = bInd / 8;

        const int NEBorder = bInd + NEOffset * std::max(0, std::min(7 - x, 7 - y) - 1);
        const int NWBorder = bInd + NWOffset * std::max(0, std::min(x, 7 - y) - 1);
        const int SEBorder = bInd + SEOffset * std::max(0, std::min(7 - x, y) - 1);
        const int SWBorder = bInd + SWOffset * std::max(0, std::min(x, y) - 1);

        // neMask generation
        ret[neMask] = GenMask(NEBorder, bInd, NEOffset, std::less_equal{});

        // nwMask generation
        ret[nwMask] = GenMask(NWBorder, bInd, NWOffset, std::less_equal{});

        // seMask generation
        ret[seMask] = GenMask(SEBorder, bInd, SEOffset, std::greater_equal{});

        // swMask generation
        ret[swMask] = GenMask(SWBorder, bInd, SWOffset, std::greater_equal{});

        return ret;
    }

    static constexpr uint64_t StripBlockingNeighbors(const uint64_t fullBoard, const MasksT&masks)
    {
        const uint64_t NWPart = ExtractLsbBit(fullBoard & masks[nwMask]);
        const uint64_t NEPart = ExtractLsbBit(fullBoard & masks[neMask]);
        const uint64_t SWPart = ExtractMsbBit(fullBoard & masks[swMask]);
        const uint64_t SEPart = ExtractMsbBit(fullBoard & masks[seMask]);
        return NWPart | NEPart | SWPart | SEPart;
    }

    // ------------------------------
    // Class inner types
    // ------------------------------

    enum maskInd
    {
        nwMask,
        neMask,
        swMask,
        seMask,
    };
};

#endif //BISHOPMAPGENERATOR_H
