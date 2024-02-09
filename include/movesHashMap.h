//
// Created by Jlisowskyy on 12/29/23.
//

#ifndef HASHMAP_H
#define HASHMAP_H

#include <array>
#include <mutex>
#include <random>
#include <format>
#include <vector>

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

        return { collisionDetected == false, maxSize*sizeof(uint64_t) };
    }

    template<class NeighborGeneratorT, class MaskInitT>
    static void FindHashParameters(const HashFuncT* const funcs, NeighborGeneratorT nGen, MaskInitT mInit) {
        movesHashMap maps[Board::BoardFields]{};
        std::vector finishedMaps(Board::BoardFields, false);
        std::mutex guard{};
        size_t fullSize{};
        size_t correctMaps{};

        // Preparing temp map to perform calculations

        for (int i = 0; i < Board::BoardFields; ++i)
            maps[i] = movesHashMap(mInit(ConvertToReversedPos(i)), funcs[i]);

        #pragma omp parallel for
        for(int i = 0; i < Board::BoardFields; ++i) {
            const int bInd = ConvertToReversedPos(i);

            // Possible neighbors generation.
            const auto [possibilities, posSize] = nGen(bInd, maps[i].masks);

            auto getNeighbors = [&](const int unused1, const std::array<uint64_t, 4>& unused2){
                return std::make_pair(std::ref(possibilities), posSize);
            };

            // First inital check for correctnes
            auto [result, size] = maps[i].IntegrityTest(getNeighbors, bInd);

            // If map is correct go to the next one
            if (result) {
                guard.lock();
                fullSize += size;
                ++correctMaps;
                finishedMaps[i] = true;
                guard.unlock();

                continue;
            };

            // Anounce thread start
            guard.lock();
            std::cout << std::format("Starting hash function parameters search on index: {} and field: {}\n",
                i, fieldStrMap.at(static_cast<Field>(1LLU << bInd)));
            guard.unlock();

            // Main Thread job - parameter searching loop
            bool wasCollision;
            size_t nSize;
            do {
                guard.lock();
                maps[i].HFunc.RollParameters();
                guard.unlock();

                const auto [rehashResult, rehashedSize] =  maps[i].IntegrityTest(getNeighbors, bInd);

                wasCollision = !rehashResult;
                nSize = rehashedSize;
            }while(wasCollision);

            guard.lock();
            // Saving results
            finishedMaps[i] = true;
            fullSize += nSize;
            ++correctMaps;

            // Printing actual parameters
            std::cout << "Actual rehashing result:\n{\n";
            for (int j = 0; j < Board::BoardFields; ++j) std::cout << '\t' << (finishedMaps[j] == true ? maps[j].HFunc : funcs[j]) << ",\n";
            std::cout << "};\n" << std::format("Current correct maps: {},\nWith size: {} bytes\n", correctMaps, fullSize);

            guard.unlock();
        }

        // Print final looking table
        std::cout << "Final parameters:\n{\n";
        for (int j = 0; j < Board::BoardFields; ++j) std::cout << '\t' << (finishedMaps[j] == true ? maps[j].HFunc : funcs[j]) << ",\n";
        std::cout << "};\n" << std::format("With size: {} bytes\n", fullSize);
    }

    template<class NeighborGeneratorT, class MaskInitT>
    static void FindCollidingIndices(const HashFuncT* const funcs, NeighborGeneratorT nGen, MaskInitT mInit) {
        std::cout << "Collision detected on indices:\n\t{";
        size_t invalidIndices{};

        for (int i = 0; i < Board::BoardFields; ++i) {
            const int bInd = ConvertToReversedPos(i);

            auto map = movesHashMap(mInit(bInd), funcs[i]);
            auto [result, _] = map.IntegrityTest(nGen, bInd);

            if (result == false) {
                std::cout << i << ", ";
                ++invalidIndices;
            }
        }
        std::cout << std::format("}}\nNumber of invalid indices: {}\n", invalidIndices);
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
