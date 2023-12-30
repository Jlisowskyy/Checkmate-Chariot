//
// Created by Jlisowskyy on 12/28/23.
//

#include <mutex>
#include <vector>
#include <random>
#include <chrono>

#include "../include/RookMap.h"

#include "../include/MoveGeneration.h"

constexpr void RookMap::_initMoves() {
    for(int i = 0; i < Board::BoardFields; ++i) {
        const int bInd = ConvertToReversedPos(i);

        // Possible neighbors generation.
        const auto [possibilities, posSize] = _genPossibleNeighbors(bInd, layer1[i]);

        for (size_t j = 0; j < posSize; ++j) {
            const uint64_t moves = _genMoves(possibilities[j], bInd);

            layer1[i][possibilities[j]] = moves;
        }
    }
}

constexpr uint64_t RookMap::_genMoves(const uint64_t neighbors, const int bInd) {
    constexpr int uBarrier = 64;
    constexpr int dBarrier = -1;
    const int lBarrier = (bInd / 8) * 8 - 1;
    const int rBarrier = lBarrier + 9;

    uint64_t moves = 0;

    // upper lines moves
    moves |= _genMovesOnLine(neighbors, bInd,
                             [](const int x){ return x + 8;},
                             [&](const int x){ return x < uBarrier; }
    );

    // lower lines moves
    moves |= _genMovesOnLine(neighbors, bInd,
                             [](const int x){ return x - 8;},
                             [&](const int x){ return x > dBarrier; }
    );

    // right line moves
    moves |= _genMovesOnLine(neighbors, bInd,
                             [](const int x){ return x + 1;},
                             [&](const int x){ return x < rBarrier; }
    );

    // left line moves
    moves |= _genMovesOnLine(neighbors, bInd,
                             [](const int x){ return x - 1;},
                             [&](const int x){ return x > lBarrier; }
    );

    return moves;
}

constexpr uint64_t RookMap::_genRMask(const int boardIndex) {
    const int lBarrier = (boardIndex / 8) * 8;
    const int barrier = lBarrier + 7;

    return GenMask(barrier, boardIndex, 1, std::less{});
}

constexpr uint64_t RookMap::_genLMask(const int boardIndex) {
    const int barrier = (boardIndex / 8) * 8;

    return GenMask(barrier, boardIndex, -1, std::greater{});
}

constexpr uint64_t RookMap::_genUMask(const int boardIndex) {
    return GenMask(56, boardIndex, 8, std::less{});
}

constexpr uint64_t RookMap::_genDMask(const int boardIndex) {
    return GenMask(7, boardIndex, -8, std::greater{});
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

constexpr uint64_t RookMap::_genModuloMask(const size_t modSize) {
    return (minMsbPossible << modSize) - 1;
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
    std::mutex guard{};

    #pragma omp parallel for
    for(int i = 0; i < Board::BoardFields; ++i) {
        if (aHashValues[i] != 0 && bHashValues[i] != 0) continue;

        const int bInd = ConvertToReversedPos(i);
        std::mt19937_64 gen64{};
        gen64.seed(std::chrono::steady_clock::now().time_since_epoch().count());

        // Possible neighbors generation.
        const auto [possibilities, posSize] = _genPossibleNeighbors(bInd, layer1[i]);

        guard.lock();
        std::cout << "Starting hash parameters search on index: " << bInd << std::endl;
        guard.unlock();

        bool wasColision = true;
        while(wasColision) {
            wasColision = false;

            layer1[i].setHashCoefs(gen64(), gen64());
            layer1[i].clear();

            for (size_t j = 0; j < posSize; ++j) {
                if (layer1[i][possibilities[j]] == 1) {
                    wasColision = true;
                    break;
                }

                layer1[i][possibilities[j]] = 1;
            }
        }

        const auto [a, b] = layer1[i].getCoefs();
        guard.lock();
        std::cout << "Finished hashing on mapIndex: " << i << " with acquired parameteres 'a': "
                << a << " 'b': " << b<< std::endl;
        guard.unlock();
    }
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
        [](const int x, const int y) { return _neighborLayoutPossibleCountOnField(x, y); },
        [](const int bInd) { return _initMasks(bInd); }
    );
}

constexpr std::array<uint64_t, movesHashMap::MasksCount> RookMap::_initMasks(const int bInd) {
    std::array<uint64_t, movesHashMap::MasksCount> ret{};

    // Mask generation.
    ret[dMask] = _genDMask(bInd);
    ret[lMask] = _genLMask(bInd);
    ret[rMask] = _genRMask(bInd);
    ret[uMask] = _genUMask(bInd);

    return ret;
}