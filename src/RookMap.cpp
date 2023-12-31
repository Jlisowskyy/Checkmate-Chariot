//
// Created by Jlisowskyy on 12/28/23.
//

#include <mutex>
#include <random>
#include <chrono>

#include "../include/RookMap.h"

#include "../include/MoveGeneration.h"

constexpr void RookMap::_initMoves() {
    MoveInitializer(layer1,
        [](const uint64_t neighbors, const int bInd) constexpr
                        { return _genMoves(neighbors, bInd); },
        [](const int bInd, const movesHashMap& record) constexpr
                        { return _genPossibleNeighbors(bInd, record); }
    );
}

constexpr uint64_t RookMap::_genMoves(const uint64_t neighbors, const int bInd) {
    constexpr int uBarrier = 64;
    constexpr int dBarrier = -1;
    const int lBarrier = (bInd / 8) * 8 - 1;
    const int rBarrier = lBarrier + 9;

    uint64_t moves = 0;

    // upper lines moves
    moves |= GenSlidingMoves(neighbors, bInd, 8,
     [&](const int x){ return x < uBarrier; }
    );

    // lower lines moves
    moves |= GenSlidingMoves(neighbors, bInd, -8,
                             [&](const int x){ return x > dBarrier; }
    );

    // right line moves
    moves |= GenSlidingMoves(neighbors, bInd, 1,
                             [&](const int x){ return x < rBarrier; }
    );

    // left line moves
    moves |= GenSlidingMoves(neighbors, bInd, -1,
                             [&](const int x){ return x > lBarrier; }
    );

    return moves;
}

constexpr std::tuple<std::array<uint64_t, RookMap::MaxRookPossibleNeighbors>, size_t> RookMap::_genPossibleNeighbors(
    const int bInd, const movesHashMap& record) {
    std::array<uint64_t, MaxRookPossibleNeighbors> ret{};
    size_t usedFields = 0;

    const int lBarrier = ((bInd >> 3) << 3) - 1;
    const int rBarrier = lBarrier + 9;
    constexpr int uBarrier = 64;
    constexpr int dBarrier = -1;

    const uint64_t bPos = 1LLU << bInd;
    for (int l = bInd; l > lBarrier; --l) {
        const uint64_t lPos = minMsbPossible << l;
        if (lPos != bPos && (record.masks[lMask] & lPos) == 0)
            continue;

        for (int r = bInd; r < rBarrier; ++r) {
            const uint64_t rPos = minMsbPossible << r;
            if (rPos != bPos && (record.masks[rMask] & rPos) == 0)
                continue;

            for (int u = bInd; u < uBarrier; u+=8) {
                const uint64_t uPos = minMsbPossible << u;
                if (uPos != bPos && (record.masks[uMask] & uPos) == 0)
                    continue;


                for (int d = bInd; d > dBarrier; d-=8) {
                    const uint64_t dPos = minMsbPossible << d;
                    if (dPos != bPos && (record.masks[dMask] & dPos) == 0)
                        continue;

                    const uint64_t neighbor = (dPos | uPos | rPos | lPos) ^ bPos;
                    ret[usedFields++] = neighbor;
                }
            }
        }
    }

    return {ret, usedFields};
}

constexpr size_t RookMap::_neighborLayoutPossibleCountOnField(const int x, const int y) {
    const int lCount = std::max(1, x);
    const int dCount = std::max(1, y);
    const int uCount = std::max(1, 7-y);
    const int rCount = std::max(1, 7-x);

    return lCount * rCount * dCount * uCount;
}


RookMap::RookMap() {
    _initMaps();
    IntegrityTest(
        [](const int bInd, const movesHashMap& map){ return _genPossibleNeighbors(bInd, map); },
        layer1
        );
    _initMoves();
}

void RookMap::FindHashParameters() {
    movesHashMap::FindHashParameters(aHashValues, bHashValues, layer1,
        [](const int bInd, const movesHashMap& record) { return _genPossibleNeighbors(bInd, record); }
    );
}

uint64_t RookMap::GetMoves(const int msbInd, const uint64_t fullBoard, const uint64_t allyBoard) const {
    const movesHashMap& rec = layer1[msbInd];

    const uint64_t uPart = ExtractLsbBit(fullBoard & rec.masks[uMask]);
    const uint64_t dPart = ExtractMsbBit(fullBoard & rec.masks[dMask]);
    const uint64_t lPart = ExtractMsbBit(fullBoard & rec.masks[lMask]);
    const uint64_t rPart = ExtractLsbBit(fullBoard & rec.masks[rMask]);
    const uint64_t closestNeighbors = uPart | dPart | lPart | rPart;
    const uint64_t moves = layer1[msbInd][closestNeighbors];

    return ClearAFromIntersectingBits(moves, allyBoard);
}

void RookMap::_initMaps() {
    MapInitializer(aHashValues, bHashValues, layer1,
        [](const int x, const int y) constexpr
            { return _neighborLayoutPossibleCountOnField(x, y); },
        [](const int bInd) constexpr
            { return _initMasks(bInd); }
    );
}

constexpr std::array<uint64_t, movesHashMap::MasksCount> RookMap::_initMasks(const int bInd) {
    std::array<uint64_t, movesHashMap::MasksCount> ret{};

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