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

    movesHashMap() = default;

    movesHashMap(const std::array<uint64_t, MasksCount>& nMasks, uint64_t a, uint64_t b, uint64_t modMask, uint64_t primeNum, size_t mapSize):
        masks{nMasks}, _a{a}, _b{b}, _moduloMask{modMask}, _primeNumber{primeNum}, _mapSize{mapSize}
    {
        _map = new uint64_t[_mapSize] { 0LLU };
    }

    movesHashMap& operator=(const movesHashMap& other){
        if (this == &other) return *this;

        delete _map;
        _a = other._a;
        _b = other._b;
        _moduloMask = other._moduloMask;
        _mapSize = other._mapSize;
        _primeNumber = other._primeNumber;
        masks = other.masks;

        _map = new uint64_t[_mapSize];

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

    void clear() const{
        for (size_t i = 0; i < _mapSize; ++i)
            _map[i] = 0;
    }

    ~movesHashMap() {
        delete[] _map;
    }

    template<class neighborGenerator>
    static void FindHashParameters(uint64_t* aHash, uint64_t* bHash, movesHashMap* maps, neighborGenerator nGen) {
        std::mutex guard{};

        #pragma omp parallel for
        for(int i = 0; i < Board::BoardFields; ++i) {
            if (aHash[i] != 0 && aHash[i] != 0) continue;

            const int bInd = ConvertToReversedPos(i);
            std::mt19937_64 gen64{};
            gen64.seed(std::chrono::steady_clock::now().time_since_epoch().count());

            // Possible neighbors generation.
            const auto [possibilities, posSize] = nGen(bInd, maps[i]);

            guard.lock();
            std::cout << "Starting hash parameters search on index: " << bInd << std::endl;
            guard.unlock();

            bool wasColision = true;
            while(wasColision) {
                wasColision = false;

                maps[i].setHashCoefs(gen64(), gen64());
                maps[i].clear();

                for (size_t j = 0; j < posSize; ++j) {
                    if (maps[i][possibilities[j]] == 1) {
                        wasColision = true;
                        break;
                    }

                    maps[i][possibilities[j]] = 1;
                }
            }

            const auto [a, b] = maps[i].getCoefs();
            guard.lock();
            aHash[i] = a;
            bHash[i] = b;

            std::cout << "aHashValue map:\n{\n";
            for (int z = 0; z < Board::BoardFields; ++z) {
                std::cout << "\t" << aHash[z] << "LLU,\n";
            }
            std::cout << "};\n";

            std::cout << "bHashValue map:\n{\n";
            for (int z = 0; z < Board::BoardFields; ++z) {
                std::cout << "\t" << bHash[z] << "LLU,\n";
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

    uint64_t* _map{};
    size_t _mapSize{};
};

#endif //HASHMAP_H
