//
// Created by Jlisowskyy on 12/29/23.
//

#ifndef HASHMAP_H
#define HASHMAP_H

#include <array>
#include <mutex>
#include <random>
#include <format>

#include "EngineTypeDefs.h"
#include "BitOperations.h"

/*          Important notes:
 *  Used HashFuncT class should have:
 *  -
 *
 *
 */

template<
    class HashFuncT,
    size_t mapAllocSize = 256
> class movesHashMap {
    // ------------------------------
    // Class creation
    // ------------------------------
public:
    static constexpr size_t MasksCount = 4;

    constexpr movesHashMap() = default;
    constexpr ~movesHashMap() = default;

    constexpr movesHashMap(const std::array<uint64_t, MasksCount>& nMasks, const typename HashFuncT::params& p ):
        masks(nMasks), HFunc(p), _map{ 0 }
    {}

    constexpr movesHashMap(const movesHashMap&) = default;
    constexpr movesHashMap(movesHashMap&&) = default;
    constexpr movesHashMap& operator=(const movesHashMap&) = default;
    constexpr movesHashMap& operator=(movesHashMap&&) = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    constexpr const uint64_t& operator[](const uint64_t neighbors) const {
        return _map[HFunc(neighbors)];
    }

    constexpr uint64_t& operator[](const uint64_t neighbors) {
        return _map[HFunc(neighbors)];
    }

    constexpr void clear() {
        _map.fill(0);
    }

    template<class neighborGenerator>
    static std::pair<bool, size_t> IntegrityTest(neighborGenerator func, movesHashMap& map, const int boardIndex) {
        const auto [possibilities, posSize] = func(boardIndex, map);

        size_t maxSize {};
        bool collisionDetected {false};

        map.clear();
        for (size_t i = 0; i < posSize; ++i) {
            if (map[possibilities[i]] == 1) {
                std::cerr << "[ ERROR ] Integrity failed on index: " << i << std::endl;
                collisionDetected = true;
                break;
            }

            map[possibilities[i]] = 1;
            if (const size_t ind = map.HFunc(possibilities[i]); ind > maxSize) maxSize = ind;
        }

        return { collisionDetected, maxSize*sizeof(uint64_t) };
    }

    template<class neighborGenerator>
    static void FindHashParameters(const typename HashFuncT::params* const params, neighborGenerator nGen) {
        movesHashMap maps[Board::BoardFields]{};
        std::mutex guard{};
        size_t fullSize{};
        size_t correctMaps{};

        for (size_t i = 0; i < Board::BoardFields; ++i)
            maps[i] = movesHashMap(params[i]);

        #pragma omp parallel for
        for(int i = 0; i < Board::BoardFields; ++i) {
            const int bInd = ConvertToReversedPos(i);

            // Possible neighbors generation.
            const auto [possibilities, posSize] = nGen(bInd, maps[i]);

            static auto getNeighbors = [&](const int x, const movesHashMap& y) -> std::pair<const auto&, size_t> {
                return { possibilities, posSize };
            };

            auto [result, size] = IntegrityTest(getNeighbors, maps[i], bInd);

            if (result) {
                const size_t roundedToCacheLine = std::ceil(static_cast<double>(size) / 64) * 64;
                fullSize += roundedToCacheLine;
                ++correctMaps;
                continue;
            };

            guard.lock();
            std::cout << std::format("Starting hash function parameters serach on index: {} and field: {}\n",
                bInd, fieldStrMap.at(static_cast<Field>(1LLU << bInd)));
            guard.unlock();

            bool wasCollision;
            size_t nSize;
            do {
                maps[i].HFunc.RollParameters();
                const auto [rehashResult, rehashedSize] = IntegrityTest(getNeighbors, maps[i], bInd);

                wasCollision = rehashResult;
                nSize = rehashedSize;
            }while(wasCollision);
            fullSize += nSize;

            guard.lock();
            std::cout << "Actual rehashing result:\n{\n";
            for (const auto& map : maps) std::cout << '\t' << map.HFunc.PrintParameters(std::cout) << ",\n";
            std::cout << "};\n" << std::format("Current corect maps: {},\nWith size: {} bytes\n", correctMaps, fullSize);
            guard.unlock();
        }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    std::array<uint64_t, MasksCount> masks{};
    uint64_t fullMask{};
    HashFuncT HFunc{};

private:

    std::array<uint64_t, mapAllocSize> _map{};
    size_t _mapSize{};
};

#endif //HASHMAP_H
