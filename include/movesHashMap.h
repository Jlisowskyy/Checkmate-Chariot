//
// Created by Jlisowskyy on 12/29/23.
//

#ifndef HASHMAP_H
#define HASHMAP_H

#include <cinttypes>
#include <array>
#include <tuple>
#include <cstdlib>

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
