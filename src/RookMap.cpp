//
// Created by Jlisowskyy on 12/28/23.
//

#include <cmath>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>

#include "../include/RookMap.h"

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

constexpr uint64_t RookMap::_genRMask(const int barrier, const int boardIndex) {
    return _genMask(barrier, boardIndex,
                    [](const int x) -> int { return x + 1; },
                    [](const int ind, const int bar) -> bool { return ind < bar; });
}

constexpr uint64_t RookMap::_genLMask(const int barrier, const int boardIndex) {
    return _genMask(barrier, boardIndex,
                    [](const int x) -> int { return x - 1; },
                    [](const int ind, const int bar) -> bool { return ind > bar; });
}

constexpr uint64_t RookMap::_genUMask(const int boardIndex) {
    return _genMask(56, boardIndex,
                    [](const int x) -> int { return x + 8; },
                    [](const int ind, const int bar) -> bool { return ind < bar; });
}

constexpr uint64_t RookMap::_genDMask(const int boardIndex) {
    return _genMask(7, boardIndex,
                    [](const int x) -> int { return x - 8; },
                    [](const int ind, const int bar) -> bool { return ind > bar; });
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

constexpr size_t RookMap::_calculatePossibleMovesCount() {
    size_t sum{};
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            sum += _neighborLayoutPossibleCountOnField(x, y);
        }
    }

    return sum;
}

RookMap::RookMap() {
    _initMaps();
    _integrityTest();
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

void RookMap::_integrityTest() {
    for (int i = 0; i < Board::BoardFields; ++i) {
        const int bInd = ConvertToReversedPos(i);
        const auto [possibilities, posSize] = _genPossibleNeighbors(bInd, layer1[i]);

        layer1[i].clear();
        for (size_t j = 0; j < posSize; ++j) {
            if (layer1[i][possibilities[j]] == 1) {
                std::cerr << "[ ERROR ] Integrity failed on index: " << i << std::endl;
                break;
            }

            layer1[i][possibilities[j]] = 1;
        }
    }
}

void RookMap::_initMaps() {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const int bInd = y*8 + x;
            const int mapInd = ConvertToReversedPos(bInd);

            const size_t minimalSize = _neighborLayoutPossibleCountOnField(x, y);
            const size_t min2Pow = std::ceil(std::log2(static_cast<double>(minimalSize)));
            const size_t mapSize = 1 << min2Pow;
            const uint64_t moduloMask = mapSize - 1;
            const uint64_t primeNumber = PrimeNumberMap.at(mapSize);
            const uint64_t a = aHashValues[mapInd];
            const uint64_t b = bHashValues[mapInd];
            const auto masks = _initMasks(bInd);

            layer1[mapInd] = movesHashMap{masks, a, b, moduloMask, primeNumber, mapSize};
        }
    }
}

std::array<uint64_t, movesHashMap::MasksCount> RookMap::_initMasks(const int bInd) {
    std::array<uint64_t, movesHashMap::MasksCount> ret{};

    // Calculating borders for valid mask generation;
    const int lBarrier = (bInd / 8) * 8;
    const int rBarrier = lBarrier + 7;

    // Mask generation.
    ret[dMask] = _genDMask(bInd);
    ret[lMask] = _genLMask(lBarrier, bInd);
    ret[rMask] = _genRMask(rBarrier, bInd);
    ret[uMask] = _genUMask(bInd);

    return ret;
}

void RookMap::_displayMasks(const movesHashMap& map) {
    static constexpr const char* names[] = { "lMask", "rMask", "uMask", "dMask" };

    for (size_t i = 0; i < movesHashMap::MasksCount; ++i) {
        std::cout << "Mask name: " << names[i] << std::endl;
        DisplayMask(map.masks[i]);
    }
}
