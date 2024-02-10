//
// Created by Jlisowskyy on 2/10/24.
//

#ifndef SPARSERANDOMGENERATOR_H
#define SPARSERANDOMGENERATOR_H

#include <random>
#include <cinttypes>

template<
    class Base64GeneratorT = std::mt19937_64
> struct SparseRandomGenerator {
    // ------------------------------
    // Class creation
    // ------------------------------

    SparseRandomGenerator() = default;
    ~SparseRandomGenerator() = default;

    SparseRandomGenerator(const SparseRandomGenerator&) = delete;
    SparseRandomGenerator& operator=(const SparseRandomGenerator&) = delete;

    explicit SparseRandomGenerator(uint64_t seed): _underlyingRandEngine(seed) {}

    // ------------------------------
    // Class interaction
    // ------------------------------

    uint64_t operator()() {
        const uint64_t rand1 = _underlyingRandEngine();
        const uint64_t rand2 = _underlyingRandEngine();
        const uint64_t rand3 = _underlyingRandEngine();

        return rand1 & rand2 & rand3;
    }

    // ------------------------------
    // class fields
    // ------------------------------
private:

    Base64GeneratorT _underlyingRandEngine{};
};

#endif //SPARSERANDOMGENERATOR_H
