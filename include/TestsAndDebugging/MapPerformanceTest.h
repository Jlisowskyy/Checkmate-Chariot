//
// Created by Jlisowskyy on 2/9/24.
//

#ifndef MAPPERFORMANCETEST_H
#define MAPPERFORMANCETEST_H

#include <string>
#include <fstream>
#include <format>
#include <iostream>
#include <vector>
#include <chrono>

#include "../BitOperations.h"

/*              IMPORTANT NOTES
 *  All files containing maps used to test must follow this scheme:
 *  - uint64_t - containing number of records - 8 bytes
 *  - records should be saved in such manner that:
 *      - uint64_t - full map - 8 bytes
 *      - uint64_t - map containing only rooks or bishops - 8 bytes
 */

class MapPerformanceTester
{
    // ------------------------------
    // inner class types
    // ------------------------------

    using RecordsPack = std::tuple<uint64_t, std::vector<uint64_t>, std::vector<uint64_t>>;
    // [size, fullMaps, figureMaps]

    // ------------------------------
    // Class creation
    // ------------------------------
public:
    MapPerformanceTester() = default;

    ~MapPerformanceTester() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template<class MapT>
    static double PerformTest(const std::string&filename, const MapT&map) noexcept(false)
    {
        auto [recordCount, fullMaps, figureMaps] = _readTestFile(filename);
        uint64_t mapReads{};
        uint64_t trulyNotRandomNumber{}; // was unsure about optimisations done when there was no use of read value

        const auto timeStart = std::chrono::steady_clock::now();

        for (size_t i = 0; i < recordCount; ++i)
        {
            const uint64_t fullMap = fullMaps[i];
            uint64_t figureMap = figureMaps[i];

            while (figureMap != 0)
            {
                const int msbPos = ExtractMsbPos(figureMap);

                const uint64_t move = map.GetMoves(msbPos, fullMap);
                trulyNotRandomNumber += move;

                ++mapReads;
                figureMap ^= maxMsbPossible >> msbPos;
            }
        }

        const auto timeStop = std::chrono::steady_clock::now();
        const double timeSpentMs = (timeStop - timeStart).count() * 1e-6;
        const double readPerMs = mapReads / timeSpentMs;

        std::cout << std::format(
                    "During the test there was {} reads in total.\nAll done in {}ms.\nWhich outputs {} reads per ms\n",
                    mapReads, timeSpentMs, readPerMs)
                << std::format("Aquired test number: {}\n", trulyNotRandomNumber);

        return readPerMs;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------
private:
    static RecordsPack _readTestFile(std::string filename)
    {
        std::ifstream stream(filename, std::ios::binary | std::ios::in);

        if (!stream)
            throw std::runtime_error(std::format("[ ERROR ] Failed when opening file: {}!\n", filename));

        uint64_t recordCount{};
        std::vector<uint64_t> fullMaps{};
        std::vector<uint64_t> figureMaps{};

        // file size read
        stream.read(reinterpret_cast<char *>(&recordCount), sizeof(uint64_t));

        fullMaps.resize(recordCount);
        figureMaps.resize(recordCount);

        if (!stream)
        {
            throw std::runtime_error(std::format("[ ERROR ] Failed when opening file: {}!\n", filename));
        }

        // records read
        for (size_t i = 0; i < recordCount; ++i)
        {
            stream.read(reinterpret_cast<char *>(&fullMaps[i]), sizeof(uint64_t));
            stream.read(reinterpret_cast<char *>(&figureMaps[i]), sizeof(uint64_t));

            if (!stream)
                throw std::runtime_error(
                    std::format("[ ERROR ] Encountered ill-formed record inside the test. Test no {} (0-based).", i));
        }

        return {recordCount, fullMaps, figureMaps};
    }
};

#endif //MAPPERFORMANCETEST_H
