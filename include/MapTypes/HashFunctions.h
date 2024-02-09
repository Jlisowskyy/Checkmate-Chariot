//
// Created by Jlisowskyy on 1/31/24.
//

#ifndef HASHFUNCTIONS_H
#define HASHFUNCTIONS_H

#include <random>
#include <chrono>
#include <format>

#include "../BitOperations.h"

template<class RandomGeneratorT = std::mt19937_64>
class Fast2PowHashFunction {
    /*                  Description
     *  Hash function optimised to be used with sizes, that are the power of 2.
     *  Returns only 32-bit values and operates on bigger 64-bit integers.
     *  Thus, this class is not portable at all.
     *
     *  Works accordingly to this formula:
     *      w = integer that size = 2^w
     *  h(v) = ((ax + b) & (2^(32+w)-1) ) >> 32 = ((ax + b) % 2^(32+w) ) / 2^32
     */

    // ------------------------------
    // Class creation
    // ------------------------------
public:

    // [a, b, size]
    using params = std::tuple<uint64_t, uint64_t, uint64_t>;

    // Note: expects setParameters to be invoked before usage
    explicit constexpr Fast2PowHashFunction() = default;

    // IMPORTANT size < 2^32
    explicit Fast2PowHashFunction(const uint64_t size){
        RollParameters();
        ChangeSize(size);
    }

    constexpr Fast2PowHashFunction(const params& p) {
        SetParameters(p);
    }

    constexpr Fast2PowHashFunction(const Fast2PowHashFunction&) = default;
    constexpr Fast2PowHashFunction(Fast2PowHashFunction&&) = default;
    constexpr Fast2PowHashFunction& operator=(const Fast2PowHashFunction&) = default;
    constexpr Fast2PowHashFunction& operator=(Fast2PowHashFunction&&) = default;

    constexpr ~Fast2PowHashFunction() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void RollParameters() {
        static RandomGeneratorT randEngine_64{
            static_cast<size_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        };

        _a = randEngine_64();
        _b = randEngine_64();
    }

    constexpr void SetParameters(const params& p) {
        const auto [a, b, size] = p;

        _a = a;
        _b = b;
        ChangeSize(size);
    }

    [[nodiscard]] constexpr params GetParams() const {
        return {_a, _b, _mask};
    }

    constexpr void ChangeSize(const size_t nSize) {
        _mask = (nSize << 32) - 1;
    }

    friend std::ostream& operator<<(std::ostream& out, const Fast2PowHashFunction& f) {
        return out << std::format("{{{}, {}, {}}}", f._a, f._b, f._getRealSize());
    }

    constexpr size_t operator()(const size_t x) const {
        return ((_a*x + _b) & _mask) >> 32;
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    [[nodiscard]] constexpr uint64_t _getRealSize() const {
        return (_mask + 1) >> 32;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    uint64_t _a;
    uint64_t _b;
    uint64_t _mask;
};

template<class RandomGeneratorT = std::mt19937_64>
class Fast2PowHashFunctionNoOffset {
    /*                  Description
     *  Modified variant of previous function, assuming offset b = 0
     *
     *  Modified formula:
     *      w = integer that size = 2^w
     *  h(v) = (ax  & (2^(32+w)-1) ) >> 32 = (ax % 2^(32+w) ) / 2^32
     */

    // ------------------------------
    // Class creation
    // ------------------------------
public:

    // [a, b, size]
    using params = std::tuple<uint64_t, uint64_t>;

    // Note: expects setParameters to be invoked before usage
    explicit constexpr Fast2PowHashFunctionNoOffset() = default;

    // IMPORTANT size < 2^32
    explicit Fast2PowHashFunctionNoOffset(const uint64_t size){
        RollParameters();
        ChangeSize(size);
    }

    constexpr Fast2PowHashFunctionNoOffset(const params& p) {
        SetParameters(p);
    }

    constexpr Fast2PowHashFunctionNoOffset(const Fast2PowHashFunctionNoOffset&) = default;
    constexpr Fast2PowHashFunctionNoOffset(Fast2PowHashFunctionNoOffset&&) = default;
    constexpr Fast2PowHashFunctionNoOffset& operator=(const Fast2PowHashFunctionNoOffset&) = default;
    constexpr Fast2PowHashFunctionNoOffset& operator=(Fast2PowHashFunctionNoOffset&&) = default;

    constexpr ~Fast2PowHashFunctionNoOffset() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void RollParameters() {
        static RandomGeneratorT randEngine_64{
            static_cast<size_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        };

        _a = randEngine_64();
    }

    constexpr void SetParameters(const params& p) {
        const auto [a, size] = p;

        _a = a;
        ChangeSize(size);
    }

    [[nodiscard]] constexpr params GetParams() const {
        return {_a, _mask};
    }

    constexpr void ChangeSize(const size_t nSize) {
        _mask = (nSize << 32) - 1;
    }

    friend std::ostream& operator<<(std::ostream& out, const Fast2PowHashFunctionNoOffset& f) {
        return out << std::format("{{{}, {}}}", f._a, f._getRealSize());
    }

    constexpr size_t operator()(const size_t x) const {
        return (_a*x & _mask) >> 32;
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    [[nodiscard]] constexpr uint64_t _getRealSize() const {
        return (_mask + 1) >> 32;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    uint64_t _a;
    uint64_t _mask;
};

template<
        bool OptimizeSecondModulo = false,
        class RandomGeneratorT = std::mt19937_64
>class BaseHashFunction {

    /*               Description
     *
     *  Works accordingly to formula:
     *  h(v) = ((a*b + b) mod prime) mod size
     *
     *  Note: formula enforces usage of 2 modulo operations
     *
     *  When optimize flag is enabled, size is expected to be power of 2,
     *  otherwise behavior is undefined.
     *
     *  Updated formula:
     *
     *  h(v) = ((a*b + b) mod prime) & sizeMask
     */

    // ------------------------------
    // Class creation
    // ------------------------------
public:

    // [a, b, prime, size]
    using params = std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>;

    // Note: expects setParameters to be invoked before usage
    constexpr BaseHashFunction() = default;

    explicit BaseHashFunction(const size_t size):
        _sizeMod{_initSizeMod(size)}
    {
        RollParameters();

        const int msbBit = ExtractMsbPos(size);
        _prime = primePer2Power[msbBit/8];
    }

    explicit BaseHashFunction(const size_t size, const size_t prime):
        _prime{prime}, _sizeMod{_initSizeMod(size)}
    {
        RollParameters();
    }

    constexpr BaseHashFunction(const params& p) {
        SetParameters(p);
    }

    constexpr BaseHashFunction(const BaseHashFunction&) = default;
    constexpr BaseHashFunction(BaseHashFunction&&) = default;
    constexpr BaseHashFunction& operator=(const BaseHashFunction&) = default;
    constexpr BaseHashFunction& operator=(BaseHashFunction&&) = default;

    constexpr ~BaseHashFunction() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void RollParameters() {
        static RandomGeneratorT randEngine_64{
            static_cast<size_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        };

        _a = randEngine_64();
        _b = randEngine_64();
    }

    [[nodiscard]] constexpr size_t GetMaxVal() const {
        return _prime-1;
    }

    constexpr void SetParameters(const params& p) {
        const auto [a, b, prime, size] = p;

        _a = a;
        _b = b;
        _prime = prime;
        _sizeMod = _initSizeMod(size);
    }

    [[nodiscard]] constexpr params GetParams() const {
        return {_a, _b, _prime, _getSize(_sizeMod)};
    }

    friend std::ostream& operator<<(std::ostream& out, const BaseHashFunction& f) {
        return out << std::format("{{{}, {}, {}, {}}}", f._a, f._b, f._prime, _getSize(f._sizeMod));
    }

    constexpr size_t operator()(const uint64_t val) const {
        if constexpr (OptimizeSecondModulo)
            return ((_a*val + _b) % _prime) & _sizeMod;
        else
            return ((_a*val + _b) % _prime) % _sizeMod;
    }

    // ------------------------------
    // Class private methods
    // ------------------------------
private:
    static constexpr size_t SIZE_ONE = 1;
    static constexpr size_t SIZE_ZERO = 0;

    static constexpr  size_t _initSizeMod(const size_t size){
        if constexpr (OptimizeSecondModulo)
            return size - 1;
        else
            return size;
    }

    static constexpr size_t _getSize(const size_t sizeMask) {
        if constexpr (OptimizeSecondModulo)
            return sizeMask + 1;
        else
            return sizeMask;
    }

    static constexpr size_t POW2FAST(const int degree) {
        return SIZE_ONE << degree;
    }

    static constexpr size_t primePer2Power[] = {
        POW2FAST(16)-17, // 0-2^8
        POW2FAST(24)-17, // 2^8-2^16
        POW2FAST(32)-17, // 2^16-2^24
        POW2FAST(40)-87, // 2^24-2^32
        POW2FAST(48)-59, // 2^32-2^40
        POW2FAST(56)-5, // 2^40-2^48
        SIZE_ZERO-59, // 2^48-2^56
    };


    // ------------------------------
    // Class fields
    // ------------------------------

    size_t _a;
    size_t _b;
    size_t _prime{1}; // Note: to prevent invalid expression on invocation on uinited funtion
    size_t _sizeMod{1}; // Note: to prevent invalid expression on invocation on uinited funtion
};

struct IdentityHash {
    IdentityHash() = default;
    explicit IdentityHash([[maybe_unused]] size_t) {}
    ~IdentityHash() = default;

    size_t operator()(const size_t x) const { return x; }
};

#endif //HASHFUNCTIONS_H
