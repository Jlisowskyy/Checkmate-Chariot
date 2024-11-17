//
// Created by Jlisowskyy on 3/24/24.
//

#include "../include/TestsAndDebugging/MapPerformanceTest.h"

#include <fstream>

MapPerformanceTester::RecordsPack MapPerformanceTester::readTestFile(std::string filename)
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
                std::format("[ ERROR ] Encountered ill-formed record inside the test. Test no {} (0-based).", i)
            );
    }

    return {recordCount, fullMaps, figureMaps};
}
