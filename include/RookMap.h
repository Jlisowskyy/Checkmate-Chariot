//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef ROOKMAP_H
#define ROOKMAP_H

#include <array>
#include <cmath>
#include <mutex>
#include <vector>
#include <random>
#include <unistd.h>

#include "EngineTypeDefs.h"

struct RookMap {
    // ------------------------------
    // Class inner types
    // ------------------------------

    struct GenRecord {
        Field field;
        uint64_t position;
        uint64_t moves;
    };

    struct mapRecord {
        uint64_t lMask{};
        uint64_t rMask{};
        uint64_t uMask{};
        uint64_t dMask{};
        uint64_t mask{};

        uint64_t a{}, b{};
        uint64_t moduloMask{};
        uint64_t primeNumber{};

        uint64_t* map{};
        size_t mapSize{};

        mapRecord() = default;
        ~mapRecord() {
            delete[] map;
        }
    };

    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

     RookMap(){
         _initMasks();
         _initMaps();
         _initMoves();
    }

    void FindHashParameters() {
        std::mutex guard{};

        #pragma omp parallel for
        for(int i = 0; i < Board::BoardFields; ++i) {
            const int bInd = 63 - i;
            std::mt19937_64 gen64{};
            gen64.seed(time(nullptr));

            // Possible neighbors generation.
            auto possibilities = _genPossibleNeighbors(bInd, layer1[i]);

            guard.lock();
            std::cout << "Starting hash parameters search on index: " << bInd << std::endl;
            guard.unlock();

            bool wasColision = true;
            while(wasColision) {
                wasColision = false;

                const uint64_t a = gen64();
                const uint64_t b = gen64();

                for (size_t j = 0; j < layer1[i].mapSize; ++j)
                    layer1[i].map[j] = 0;

                const size_t range = std::get<1>(possibilities);
                for (size_t j = 0; j < range; ++j) {
                    const size_t hash = ((a * possibilities[j] + b) % layer1[i].primeNumber) & layer1[i].moduloMask;

                    if (layer1[i].map[hash] == 1) {
                        wasColision = true;
                        break;
                    }

                    layer1[i].map[hash] = 1;
                }

                if (!wasColision) {
                    layer1[i].a = a;
                    layer1[i].b = b;
                }
            }

            guard.lock();
            std::cout << "Finished hashing on mapIndex: " << i << " with acquired parameteres 'a': "
                << layer1[i].a << " 'b': " << layer1[i].b << std::endl;
            guard.unlock();
        }
    }

    [[nodiscard]] uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const {
        const size_t hash = ((layer1[msbInd].a * (fullBoard & layer1[msbInd].mask) + layer1[msbInd].b)
            % layer1[msbInd].primeNumber) & layer1[msbInd].moduloMask;

        return layer1[msbInd].map[hash];
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    void _initMoves() {

    }

    void _initMaps() {
        for (size_t y = 0; y < 8; ++y) {
            for (size_t x = 0; x < 8; ++x) {
                const size_t bInd = y*8 + x;
                const size_t mapInd = 63 - bInd;

                const size_t minimalSize = _neighborLayoutPossibleCountOnField(static_cast<int>(x), static_cast<int>(y));
                const size_t min2Pow = std::ceil(std::log2(static_cast<double>(minimalSize)));
                layer1[mapInd].mapSize = 1 << min2Pow;
                layer1[mapInd].moduloMask = (1 << min2Pow) - 1;
                layer1[mapInd].primeNumber = PrimeNumberMap.at(1  << min2Pow);
                layer1[mapInd].map = new uint64_t[1 << min2Pow]{0};
            }
        }
    }

    constexpr void _initMasks(){
        for(int i = 0; i < Board::BoardFields; ++i) {
            const int bInd = 63 - i;

            // Calculating borders for valid mask generation;
            const int lBarrier = (bInd / 8) * 8;
            const int rBarrier = lBarrier + 7;

            // Mask generation.
            layer1[i].dMask = _genDMask(bInd);
            layer1[i].lMask = _genLMask(lBarrier, bInd);
            layer1[i].rMask = _genRMask(rBarrier, bInd);
            layer1[i].uMask = _genUMask(bInd);
            layer1[i].mask = layer1[i].lMask | layer1[i].rMask | layer1[i].uMask | layer1[i].dMask;
        }
    }

    template<class incFunc, class boundryCheckFunc>
    constexpr static uint64_t _genMask(int barrier, int boardIndex, incFunc inc, boundryCheckFunc boundryCheck) {
        uint64_t mask = 0;

        while(boundryCheck(boardIndex = inc(boardIndex), barrier))
            mask |= (1LLU<<boardIndex);

        return mask;
    }

    constexpr static uint64_t _genRMask(const int barrier, const int boardIndex) {
        return _genMask(barrier, boardIndex,
            [](const int x) -> int { return x + 1; },
            [](const int ind, const int bar) -> bool { return ind < bar; });
    }

    constexpr static uint64_t _genLMask(const int barrier, const int boardIndex) {
        return _genMask(barrier, boardIndex,
            [](const int x) -> int { return x - 1; },
            [](const int ind, const int bar) -> bool { return ind > bar; });
    }

    constexpr static uint64_t _genUMask(const int boardIndex) {
        return _genMask(56, boardIndex,
            [](const int x) -> int { return x + 8; },
            [](const int ind, const int bar) -> bool { return ind < bar; });
    }

    constexpr static uint64_t _genDMask(const int boardIndex) {
        return _genMask(7, boardIndex,
            [](const int x) -> int { return x - 8; },
            [](const int ind, const int bar) -> bool { return ind > bar; });
    }

    static constexpr size_t MaxRookPossibleNeighbors = 144;
    static std::array<uint64_t, MaxRookPossibleNeighbors> _genPossibleNeighbors(const int bInd, const mapRecord& record) {
        std::array<uint64_t, MaxRookPossibleNeighbors> ret{};
        size_t usedFields = 0;

        const int lBarrier = ((bInd >> 3) << 3) - 1;
        const int rBarrier = lBarrier + 9;
        constexpr int uBarrier = 64;
        constexpr int dBarrier = -1;

        const uint64_t bPos = 1LLU << bInd;
        for (int l = bInd; l > lBarrier; --l) {
            const uint64_t lPos = 1LLU << l;
            if (lPos != bPos && (record.lMask & lPos) == 0)
                continue;

            for (int r = bInd; r < rBarrier; ++r) {
                const uint64_t rPos = 1LLU << r;
                if (rPos != bPos && (record.rMask & rPos) == 0)
                    continue;

                for (int u = bInd; u < uBarrier; u+=8) {
                    const uint64_t uPos = 1LLU << u;
                    if (uPos != bPos && (record.uMask & uPos) == 0)
                        continue;


                    for (int d = bInd; d > dBarrier; d-=8) {
                        const uint64_t dPos = 1LLU << d;
                        if (dPos != bPos && (record.dMask & dPos) == 0)
                            continue;

                        const uint64_t neighbor = (dPos | uPos | rPos | lPos) ^ bInd;
                        ret[usedFields++] = neighbor;
                    }
                }
            }
        }

        return ret;
    }

    static constexpr uint64_t _genModuloMask(const size_t modSize) {
        return (1LLU << modSize) - 1;
    }

    static size_t _neighborLayoutPossibleCountOnField(const int x, const int y){
        const int lCount = std::max(1, x);
        const int dCount = std::max(1, y);
        const int uCount = std::max(1, 7-y);
        const int rCount = std::max(1, 7-x);

        return lCount * rCount * dCount * uCount;
    }

    static size_t _calculatePossibleMovesCount() {
        size_t sum{};
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                sum += _neighborLayoutPossibleCountOnField(x, y);
            }
        }

        return sum;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    mapRecord layer1[Board::BoardFields] {};
};



#endif //ROOKMAP_H
