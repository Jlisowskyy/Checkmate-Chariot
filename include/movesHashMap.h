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
 *  Used HashFuncT class must have defined:
 *  - size_t operator()(uint64_t) - which calculates value of hash function for given uint64_t key
 *  - copy constructor
 *  - void RollParameters() - rolls internal state of the function randomly, allows searching for optimal results\
 *  - friend std::ostream& operator<<(std::ostream& out, const funcT& f) const - prints internal state of the function
 *    allows logging results to file or even stdout.
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

    constexpr movesHashMap(const std::array<uint64_t, MasksCount>& nMasks, const HashFuncT& func ):
        masks(nMasks), HFunc(func), _map{ 0 }
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

    constexpr void InitFullMask() {
        fullMask = masks[0] | masks[1] | masks[2] | masks[3];
    }

    constexpr void clear() {
        _map.fill(0);
    }

    template<class neighborGenerator, bool ShouldSignal = false>
    std::pair<bool, size_t> IntegrityTest(neighborGenerator func, const int boardIndex) {
        const auto [possibilities, posSize] = func(boardIndex, masks);

        size_t maxSize {};
        bool collisionDetected {false};

        clear();
        for (size_t i = 0; i < posSize; ++i) {
            if ((*this)[possibilities[i]] == 1) {
                if constexpr (ShouldSignal) std::cerr << "[ ERROR ] Integrity failed on index: " << i << std::endl;
                collisionDetected = true;
                break;
            }

            (*this)[possibilities[i]] = 1;
            if (const size_t ind = HFunc(possibilities[i]); ind > maxSize) maxSize = ind;
        }

        return { !collisionDetected, maxSize*sizeof(uint64_t) };
    }

    template<class NeighborGeneratorT, class MaskInitT>
    static void FindHashParameters(const HashFuncT* const funcs, NeighborGeneratorT nGen, MaskInitT mInit) {
        movesHashMap maps[Board::BoardFields]{};
        std::mutex guard{};
        size_t fullSize{};
        size_t correctMaps{};

        #pragma omp parallel for
        for(int i = 0; i < Board::BoardFields; ++i) {
            const int bInd = ConvertToReversedPos(i);

            // Preparing temp map to perform calculations
            maps[i] = movesHashMap(mInit(bInd), funcs[i]);

            // Possible neighbors generation.
            const auto [possibilities, posSize] = nGen(bInd, maps[i].masks);

            static auto getNeighbors = [&](const int x, const std::array<uint64_t, 4>& m){
                return std::make_pair(possibilities, posSize);
            };

            auto [result, size] = maps[i].IntegrityTest(getNeighbors, bInd);

            if (result) {
                const size_t roundedToCacheLine = std::ceil(static_cast<double>(size) / 64) * 64;
                fullSize += roundedToCacheLine;
                ++correctMaps;
                continue;
            };

            guard.lock();
            std::cout << std::format("Starting hash function parameters search on index: {} and field: {}\n",
                bInd, fieldStrMap.at(static_cast<Field>(1LLU << bInd)));
            guard.unlock();

            bool wasCollision;
            size_t nSize;
            do {
                maps[i].HFunc.RollParameters();
                const auto [rehashResult, rehashedSize] =  maps[i].IntegrityTest(getNeighbors, bInd);

                wasCollision = !rehashResult;
                nSize = rehashedSize;
            }while(wasCollision);
            fullSize += nSize;

            guard.lock();
            std::cout << "Actual rehashing result:\n{\n";
            for (const auto& map : maps) std::cout << '\t' << map.HFunc << ",\n";
            std::cout << "};\n" << std::format("Current correct maps: {},\nWith size: {} bytes\n", correctMaps, fullSize);
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
};

#endif //HASHMAP_H
