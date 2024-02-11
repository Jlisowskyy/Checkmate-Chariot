//
// Created by Jlisowskyy on 1/1/24.
//

#ifndef ROOKMAPGENERATOR_H
#define ROOKMAPGENERATOR_H

#include <array>

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "MoveGeneration.h"

class RookMapGenerator {
public:
    static constexpr size_t MaxRookPossibleNeighborsWoutOverlap = 144;
    static constexpr size_t MaxRookPossibleNeighborsWithOverlap = 4096;
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

    [[nodiscard]] constexpr static MasksT InitMasks(const int bInd){
        std::array<uint64_t, DirectedMaskCount> ret{};

        // Mask generation.
        const int lBarrier = (bInd / 8) * 8;
        const int rBarrier = lBarrier + 7;

        // Lower vertical line
        ret[dMask] = GenMask(7, bInd, -8, std::greater{});

        // Left Horizontal line
        ret[lMask] = GenMask(lBarrier, bInd, -1, std::greater{});

        // Right horizontal line
        ret[rMask] = GenMask(rBarrier, bInd, 1, std::less{});

        // Upper vertical line
        ret[uMask] = GenMask(56, bInd, 8, std::less{});

        return ret;
    }

    [[nodiscard]] constexpr static uint64_t GenMoves(const uint64_t neighborsWoutOverlap, const int bInd){
        constexpr int uBarrier = 64;
        constexpr int dBarrier = -1;
        const int lBarrier = (bInd / 8) * 8 - 1;
        const int rBarrier = lBarrier + 9;

        uint64_t moves = 0;

        // upper lines moves
        moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, 8,
         [&](const int x){ return x < uBarrier; }
        );

        // lower lines moves
        moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, -8,
                                 [&](const int x){ return x > dBarrier; }
        );

        // right line moves
        moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, 1,
                                 [&](const int x){ return x < rBarrier; }
        );

        // left line moves
        moves |= GenSlidingMoves(neighborsWoutOverlap, bInd, -1,
                                 [&](const int x){ return x > lBarrier; }
        );

        return moves;
    }

    [[nodiscard]] constexpr static std::tuple<std::array<uint64_t, MaxRookPossibleNeighborsWoutOverlap>, size_t>
        GenPossibleNeighborsWoutOverlap(const int bInd, const MasksT& masks)
    {
        std::array<uint64_t, MaxRookPossibleNeighborsWoutOverlap> ret{};
        size_t usedFields = 0;

        const int lBarrier = ((bInd >> 3) << 3) - 1;
        const int rBarrier = lBarrier + 9;
        constexpr int uBarrier = 64;
        constexpr int dBarrier = -1;

        const uint64_t bPos = 1LLU << bInd;
        for (int l = bInd; l > lBarrier; --l) {
            const uint64_t lPos = minMsbPossible << l;
            if (lPos != bPos && (masks[lMask] & lPos) == 0)
                continue;

            for (int r = bInd; r < rBarrier; ++r) {
                const uint64_t rPos = minMsbPossible << r;
                if (rPos != bPos && (masks[rMask] & rPos) == 0)
                    continue;

                for (int u = bInd; u < uBarrier; u+=8) {
                    const uint64_t uPos = minMsbPossible << u;
                    if (uPos != bPos && (masks[uMask] & uPos) == 0)
                        continue;


                    for (int d = bInd; d > dBarrier; d-=8) {
                        const uint64_t dPos = minMsbPossible << d;
                        if (dPos != bPos && (masks[dMask] & dPos) == 0)
                            continue;

                        const uint64_t neighbor = (dPos | uPos | rPos | lPos) & ~bPos;
                        ret[usedFields++] = neighbor;
                    }
                }
            }
        }

        return {ret, usedFields};
    }

    [[nodiscard]] constexpr static std::tuple<std::array<uint64_t, MaxRookPossibleNeighborsWithOverlap>, size_t>
        GenPossibleNeighborsWithOverlap(const MasksT& masks)
    {
        std::array<uint64_t, MaxRookPossibleNeighborsWithOverlap> ret{};
        const uint64_t fullMask = masks[uMask] | masks[dMask] | masks[rMask] | masks[lMask];

        size_t usedFields = GenerateBitPermutations(fullMask, ret);

        return {ret, usedFields};
    }

    [[nodiscard]] static constexpr size_t PossibleNeighborWoutOverlapCountOnField(const int x, const int y)
    {
        const int lCount = std::max(1, x);
        const int dCount = std::max(1, y);
        const int uCount = std::max(1, 7-y);
        const int rCount = std::max(1, 7-x);

        return lCount * rCount * dCount * uCount;
    }

    static constexpr uint64_t StripBlockingNeighbors(const uint64_t fullBoard, const MasksT& masks) {
        const uint64_t uPart = ExtractLsbBit(fullBoard & masks[uMask]);
        const uint64_t dPart = ExtractMsbBit(fullBoard & masks[dMask]);
        const uint64_t lPart = ExtractMsbBit(fullBoard & masks[lMask]);
        const uint64_t rPart = ExtractLsbBit(fullBoard & masks[rMask]);
        return  uPart | dPart | lPart | rPart;
    }

    // ------------------------------
    // Class inner types
    // ------------------------------

    enum maskInd
    {
        lMask,
        rMask,
        uMask,
        dMask,
    };

};

#endif //ROOKMAPGENERATOR_H
