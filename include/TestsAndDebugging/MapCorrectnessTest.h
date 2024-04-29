//
// Created by Jlisowskyy on 2/9/24.
//

#ifndef MAPCORRECTNESSTEST_H
#define MAPCORRECTNESSTEST_H

#include <cinttypes>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../BitOperations.h"
#include "../EngineUtils.h"

/*              IMPORTANT NOTES
 *  All files containing maps used to test must follow this scheme:
 *  - uint64_t - containing number of records - 8 bytes
 *  - records should be saved in such manner that:
 *      - uint64_t - full map - 8 bytes
 *      - uint64_t - map containing only rooks or bishops - 8 bytes
 *      - uint8_t - contains number of following results, all should be sorted from the highest board index to lowest -
 * 1 byte
 *      - uint64_t - move results for each figure from the highest board index to lowest - 8 bytes each
 */

// Class used to perform tests on moves maps, based on maps stored inside files
class MapCorrectnessTester
{
    // ------------------------------
    // inner class types
    // ------------------------------

    using RecordsPack =
        std::tuple<uint64_t, std::vector<uint64_t>, std::vector<uint64_t>, std::vector<std::vector<uint64_t>>>;
    // [size, fullMaps, figureMaps]

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    MapCorrectnessTester() = default;

    ~MapCorrectnessTester() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <class MapT> static void PerformTest(const std::string &filename, const MapT &map) noexcept(false);

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    static RecordsPack _readTestFile(std::string filename);
};

template <class MapT>
void MapCorrectnessTester::PerformTest(const std::string &filename, const MapT &map) noexcept(false)
{
    auto [recordCount, fullMaps, figureMaps, correctMoves] = _readTestFile(filename);
    uint64_t errorCount{};
    uint64_t moveCount{};
    uint64_t lastErrorMove{};
    uint64_t lastErrorMoveCorrectOne{};
    uint64_t lastErrorMap{};
    uint64_t lastFigPos{};

    for (size_t i = 0; i < recordCount; ++i)
    {
        const uint64_t fullMap = fullMaps[i];
        uint64_t figureMap     = figureMaps[i];

        int figNum{};
        while (figureMap != 0)
        {
            const int msbPos = ExtractMsbPos(figureMap);
            ++moveCount;

            if (const uint64_t move = map.GetMoves(msbPos, fullMap); move != correctMoves[i][figNum])
            {
                ++errorCount;
                lastErrorMove           = move;
                lastErrorMoveCorrectOne = correctMoves[i][figNum];
                lastErrorMap            = fullMap;
                lastFigPos              = maxMsbPossible >> msbPos;
            }

            ++figNum;
            figureMap ^= maxMsbPossible >> msbPos;
        }
    }

    // Test summary
    std::cout << "Test finished!!\n" << std::format("Total errors encountered: {}\n", errorCount);
    std::cout << std::format("Processed records: {}\nTotally checked: {} moves\n", recordCount, moveCount);
    if (errorCount)
    {
        std::cout << std::format("________Last error move on field {}:\n\n", ConvertToStrPos(lastFigPos));
        DisplayMask(lastErrorMove);
        std::cout << "________Correct move:\n\n";
        DisplayMask(lastErrorMoveCorrectOne);
        std::cout << "________On map:\n\n";
        DisplayMask(lastErrorMap);
    }
}

#endif // MAPCORRECTNESSTEST_H
