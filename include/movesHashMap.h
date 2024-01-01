//
// Created by Jlisowskyy on 12/29/23.
//

#ifndef HASHMAP_H
#define HASHMAP_H

#include <cinttypes>
#include <array>
#include <tuple>
#include <mutex>
#include <random>

#include "EngineTypeDefs.h"
#include "BitOperations.h"

class movesHashMap {
    // ------------------------------
    // Class creation
    // ------------------------------
public:
    static constexpr size_t MasksCount = 4;

    constexpr movesHashMap() = default;
    constexpr ~movesHashMap() = default;

    constexpr movesHashMap(const std::array<uint64_t, MasksCount>& nMasks, uint64_t a, uint64_t b, uint64_t modMask, uint64_t primeNum, size_t mapSize):
        masks{nMasks}, _a{a}, _b{b}, _moduloMask{modMask}, _primeNumber{primeNum}, _mapSize{mapSize}, _map{ 0 }
    {}

    movesHashMap& operator=(const movesHashMap& other){
        if (this == &other) return *this;

        _a = other._a;
        _b = other._b;
        _moduloMask = other._moduloMask;
        _mapSize = other._mapSize;
        _primeNumber = other._primeNumber;
        masks = other.masks;
        _map = other._map;

        return *this;
    }

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] constexpr uint64_t getHash(const uint64_t neighbors) const {
        return ((_a * neighbors + _b) % _primeNumber) & _moduloMask;
    }

    constexpr const uint64_t& operator[](const uint64_t neighbors) const {
        return _map[getHash(neighbors)];
    }

    constexpr uint64_t& operator[](const uint64_t neighbors) {
        return _map[getHash(neighbors)];
    }
    void setHashCoefs(const uint64_t a, const uint64_t b) { _a = a; _b = b; }

    [[nodiscard]] std::tuple<uint64_t, uint64_t> getCoefs() const { return {_a, _b}; }

    void clear() {
        for (size_t i = 0; i < _mapSize; ++i)
            _map[i] = 0;
    }

    template<class neighborGenerator>
    static void FindHashParameters(const uint64_t* aHash, const uint64_t* bHash, const movesHashMap* maps, neighborGenerator nGen) {
        uint64_t myAHash[Board::BoardFields]{};
        uint64_t myBHash[Board::BoardFields]{};
        movesHashMap myMaps[Board::BoardFields]{};
        std::mutex guard{};

        for (size_t i = 0; i < Board::BoardFields; ++i) {
            myAHash[i] = aHash[i];
            myBHash[i] = bHash[i];
            myMaps[i] = maps[i];
        }

        IntegrityTest(nGen, maps);

        #pragma omp parallel for
        for(int i = 0; i < Board::BoardFields; ++i) {
            if (myAHash[i] != 0 && myBHash[i] != 0) continue;

            const int bInd = ConvertToReversedPos(i);
            std::mt19937_64 gen64{};
            gen64.seed(std::chrono::steady_clock::now().time_since_epoch().count());

            // Possible neighbors generation.
            const auto [possibilities, posSize] = nGen(bInd, myMaps[i]);

            guard.lock();
            std::cout << "Starting hash parameters search on index: " << bInd << std::endl;
            guard.unlock();

            bool wasColision = true;
            while(wasColision) {
                wasColision = false;

                myMaps[i].setHashCoefs(gen64(), gen64());
                myMaps[i].clear();

                for (size_t j = 0; j < posSize; ++j) {
                    if (myMaps[i][possibilities[j]] == 1) {
                        wasColision = true;
                        break;
                    }

                    myMaps[i][possibilities[j]] = 1;
                }
            }

            const auto [a, b] = myMaps[i].getCoefs();
            guard.lock();
            myAHash[i] = a;
            myBHash[i] = b;

            std::cout << "aHashValue map:\n{\n";
            for (const auto el : myAHash) {
                std::cout << "\t" << el << "LLU,\n";
            }
            std::cout << "};\n";

            std::cout << "bHashValue map:\n{\n";
            for (const auto el : myBHash)  {
                std::cout << "\t" << el << "LLU,\n";
            }
            std::cout << "};\n";

            guard.unlock();
        }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    std::array<uint64_t, MasksCount> masks{};
private:
    uint64_t _a{}, _b{};
    uint64_t _moduloMask{};
    uint64_t _primeNumber{};

    std::array<uint64_t, 256> _map{};
    size_t _mapSize{};
};

#endif //HASHMAP_H
