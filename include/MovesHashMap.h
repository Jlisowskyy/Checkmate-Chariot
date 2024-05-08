//
// Created by Jlisowskyy on 12/29/23.
//

#ifndef HASHMAP_H
#define HASHMAP_H

#include <array>
#include <format>
#include <mutex>
#include <vector>

#include "BitOperations.h"
#include "EngineUtils.h"
#include "Interface/Logger.h"

static inline Logger ParameterSearchLogger{};

/*          Important notes:
 *  Used HashFuncT class must have defined:
 *  - size_t operator()(uint64_t) - which calculates value of hash function for given uint64_t key
 *  - copy constructor
 *  - void RollParameters() - rolls internal state of the function randomly, allows searching for optimal results\
 *  - friend std::ostream& operator<<(std::ostream& out, const funcT& f) const - prints internal state of the function
 *    allows logging results to file or even stdout.
 *
 */

template <class HashFuncT, size_t mapAllocSize = 256> class MovesHashMap
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    static constexpr size_t MasksCount = 4;

    constexpr MovesHashMap() = default;

    constexpr ~MovesHashMap() = default;

    constexpr MovesHashMap(const std::array<uint64_t, MasksCount> &nMasks, const HashFuncT &func)
        : masks(nMasks), HFunc(func), _map{0}
    {
    }

    constexpr MovesHashMap(const MovesHashMap &) = default;

    constexpr MovesHashMap(MovesHashMap &&) = default;

    constexpr MovesHashMap &operator=(const MovesHashMap &) = default;

    constexpr MovesHashMap &operator=(MovesHashMap &&) = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    constexpr const uint64_t &operator[](const uint64_t neighbors) const { return _map[HFunc(neighbors)]; }

    constexpr uint64_t &operator[](const uint64_t neighbors) { return _map[HFunc(neighbors)]; }

    constexpr void InitFullMask() { fullMask = masks[0] | masks[1] | masks[2] | masks[3]; }

    constexpr void clear() { _map.fill(EmptyField); }

    template <
        class NeighborGeneratorT, uint64_t (*stripFunction)(uint64_t, const std::array<uint64_t, 4> &) = nullptr,
        bool ShouldSignal = false>
    std::pair<bool, size_t> IntegrityTest(NeighborGeneratorT func, int boardIndex);

    template <
        class NeighborGeneratorT, class MaskInitT,
        uint64_t (*stripFunction)(uint64_t, const std::array<uint64_t, 4> &) = nullptr>
    static void FindHashParameters(const HashFuncT *funcs, NeighborGeneratorT nGen, MaskInitT mInit);

    template <
        class NeighborGeneratorT, class MaskInitT,
        uint64_t (*stripFunction)(uint64_t, const std::array<uint64_t, 4> &) = nullptr>
    static void FindCollidingIndices(const HashFuncT *funcs, NeighborGeneratorT nGen, MaskInitT mInit);

    // ------------------------------
    // Class fields
    // ------------------------------

    std::array<uint64_t, MasksCount> masks{};
    uint64_t fullMask{};
    HashFuncT HFunc{};

    private:
    static constexpr uint64_t EmptyField = ~0ULL;

    std::array<uint64_t, mapAllocSize> _map{};
};

template <class HashFuncT, size_t mapAllocSize>
template <
    class NeighborGeneratorT, uint64_t (*stripFunction)(uint64_t, const std::array<uint64_t, 4> &), bool ShouldSignal>
std::pair<bool, size_t>
MovesHashMap<HashFuncT, mapAllocSize>::IntegrityTest(NeighborGeneratorT func, const int boardIndex)
{
    const auto [possibilities, posSize] = func(boardIndex, masks);

    size_t maxSize{};
    bool collisionDetected{false};

    clear();
    for (size_t i = 0; i < posSize; ++i)
    {
        uint64_t record;

        if constexpr (stripFunction)
            record = stripFunction(possibilities[i], masks);
        else
            record = possibilities[i];

        if (const uint64_t oldRecord = (*this)[possibilities[i]]; oldRecord != EmptyField && oldRecord != record)
        {
            if constexpr (ShouldSignal)
                ParameterSearchLogger << std::format("[ ERROR ] Integrity failed on index: {}", i);

            collisionDetected = true;
            break;
        }

        (*this)[possibilities[i]] = record;
        if (const size_t ind = HFunc(possibilities[i]); ind > maxSize)
            maxSize = ind;
    }

    return {collisionDetected == false, maxSize * sizeof(uint64_t)};
}

template <class HashFuncT, size_t mapAllocSize>
template <
    class NeighborGeneratorT, class MaskInitT, uint64_t (*stripFunction)(uint64_t, const std::array<uint64_t, 4> &)>
void MovesHashMap<HashFuncT, mapAllocSize>::FindHashParameters(
    const HashFuncT *const funcs, NeighborGeneratorT nGen, MaskInitT mInit
)
{
    MovesHashMap maps[Board::BitBoardFields]{};
    std::vector finishedMaps(Board::BitBoardFields, false);
    std::mutex guard{};
    size_t fullSize{};
    size_t correctMaps{};

    // Preparing the temp map to perform calculations
    for (int i = 0; i < static_cast<int>(Board::BitBoardFields); ++i)
        maps[i] = MovesHashMap(mInit(ConvertToReversedPos(i)), funcs[i]);

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(Board::BitBoardFields); ++i)
    {
        const int bInd = ConvertToReversedPos(i);

        // Possible neighbors generation.
        const auto tpl1    = nGen(bInd, maps[i].masks);
        auto possibilities = std::get<0>(tpl1);
        auto posSize       = std::get<1>(tpl1);

        auto getNeighbors = [&]([[maybe_unused]] const int, [[maybe_unused]] const std::array<uint64_t, 4>)
        {
            return std::make_pair(std::ref(possibilities), posSize);
        };

        // First initial check for correctness
        auto tpl2   = maps[i].template IntegrityTest<decltype(getNeighbors), stripFunction>(getNeighbors, bInd);
        auto result = std::get<0>(tpl2);
        auto size   = std::get<1>(tpl2);
        // If the map is correct, go to the next one
        if (result)
        {
            guard.lock();
            fullSize += size;
            ++correctMaps;
            finishedMaps[i] = true;
            guard.unlock();

            continue;
        };

        // Announce thread start
        guard.lock();
        ParameterSearchLogger.Log(std::format(
            "Starting hash function parameters search on index: {} and field: {}\n", i, ConvertToStrPos(bInd)
        ));
        guard.unlock();

        // Main Thread job - parameter searching loop
        bool wasCollision;
        size_t nSize;
        do
        {
            guard.lock();
            maps[i].HFunc.RollParameters();
            guard.unlock();

            const auto tpl3 = maps[i].template IntegrityTest<decltype(getNeighbors), stripFunction>(getNeighbors, bInd);

            auto rehashResult = std::get<0>(tpl3);
            auto rehashedSize = std::get<1>(tpl3);

            wasCollision = !rehashResult;
            nSize        = rehashedSize;
        } while (wasCollision);

        // Saving results
        guard.lock();
        finishedMaps[i] = true;
        fullSize += nSize;
        ++correctMaps;

        // Printing actual parameters
        ParameterSearchLogger.Log("Actual rehashing result:\n{");
        for (size_t j = 0; j < Board::BitBoardFields; ++j)
            ParameterSearchLogger << '\t' << (finishedMaps[j] ? maps[j].HFunc : funcs[j]) << ",\n";
        ParameterSearchLogger.Log(
            std::format("}};\nCurrent correct maps: {},\nWith size: {} bytes", correctMaps, fullSize)
        );

        guard.unlock();
    }

    // Print final looking table
    ParameterSearchLogger.Log("Actual rehashing result:\n{");
    for (size_t j = 0; j < Board::BitBoardFields; ++j)
        ParameterSearchLogger << '\t' << (finishedMaps[j] ? maps[j].HFunc : funcs[j]) << ",\n";
    ParameterSearchLogger.Log(std::format("}};\nCurrent correct maps: {},\nWith size: {} bytes", correctMaps, fullSize)
    );
}

template <class HashFuncT, size_t mapAllocSize>
template <
    class NeighborGeneratorT, class MaskInitT, uint64_t (*stripFunction)(uint64_t, const std::array<uint64_t, 4> &)>
void MovesHashMap<HashFuncT, mapAllocSize>::FindCollidingIndices(
    const HashFuncT *const funcs, NeighborGeneratorT nGen, MaskInitT mInit
)
{
    ParameterSearchLogger << "Collision detected on indices:\n\t{ ";
    size_t invalidIndices{};

    for (int i = 0; i < static_cast<int>(Board::BitBoardFields); ++i)
    {
        const int bInd = ConvertToReversedPos(i);

        auto map         = MovesHashMap(mInit(bInd), funcs[i]);
        auto [result, _] = map.template IntegrityTest<NeighborGeneratorT, stripFunction>(nGen, bInd);

        if (result == false)
        {
            ParameterSearchLogger << std::format("{}, ", i);
            ++invalidIndices;
        }
    }
    ParameterSearchLogger.Log(std::format("}}\nNumber of invalid indices: {}\n", invalidIndices));
}

#endif // HASHMAP_H
