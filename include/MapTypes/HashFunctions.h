//
// Created by Jlisowskyy on 1/31/24.
//

#ifndef HASHFUNCTIONS_H
#define HASHFUNCTIONS_H

#include <chrono>
#include <format>
#include <random>

#include "../BitOperations.h"

template <class RandomGeneratorT = std::mt19937_64> class FancyMagicHashFunction
{
    /*               Description
     *
     *  Works accordingly to formula:
     *  h(v) = magic * v << offset
     *  where magic is 64-bit unsigned integer and offset is int in range 0-63.
     */

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    // [magic, shift]
    using params = std::tuple<uint64_t, uint64_t>;

    // Note: expects setParameters to be invoked before usage
    constexpr FancyMagicHashFunction() = default;

    explicit FancyMagicHashFunction(const uint64_t offset) : _offset(offset) { RollParameters(); }

    constexpr FancyMagicHashFunction(const params &p) { SetParameters(p); }

    constexpr FancyMagicHashFunction(const FancyMagicHashFunction &) = default;

    constexpr FancyMagicHashFunction(FancyMagicHashFunction &&) = default;

    constexpr FancyMagicHashFunction &operator=(const FancyMagicHashFunction &) = default;

    constexpr FancyMagicHashFunction &operator=(FancyMagicHashFunction &&) = default;

    constexpr ~FancyMagicHashFunction() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void RollParameters()
    {
        static RandomGeneratorT randEngine_64{
            static_cast<size_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        };

        _magic = randEngine_64();
    }

    constexpr void SetParameters(const params &p)
    {
        const auto [magic, offset] = p;

        _magic  = magic;
        _offset = offset;
    }

    [[nodiscard]] constexpr params GetParams() const { return {_magic, _offset}; }

    friend std::ostream &operator<<(std::ostream &out, const FancyMagicHashFunction &f)
    {
        return out << std::format("_hashFuncT(std::make_tuple({}LLU, {}))", f._magic, f._offset);
    }

    constexpr size_t operator()(const uint64_t val) const { return (val * _magic) >> (64 - _offset); }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    uint64_t _magic;
    uint64_t _offset;
};


#endif // HASHFUNCTIONS_H
