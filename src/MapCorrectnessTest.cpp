//
// Created by Jlisowskyy on 3/24/24.
//

#include "../include/TestsAndDebugging/MapCorrectnessTest.h"

MapCorrectnessTester::RecordsPack MapCorrectnessTester::readTestFile(std::string filename)
{
    std::fstream stream(filename, std::fstream::binary | std::fstream::in);

    if (!stream)
        throw std::runtime_error(std::format("[ ERROR ] Failed when opening file: {}!\n", filename));

    uint64_t recordCount{};
    std::vector<uint64_t> fullMaps{};
    std::vector<uint64_t> figureMaps{};
    std::vector<std::vector<uint64_t>> correctResults{};

    // file size read
    stream.read(reinterpret_cast<char *>(&recordCount), sizeof(uint64_t));

    fullMaps.resize(recordCount);
    figureMaps.resize(recordCount);
    correctResults.resize(recordCount);

    // records read
    for (size_t i = 0; i < recordCount; ++i)
    {
        stream.read(reinterpret_cast<char *>(&fullMaps[i]), sizeof(uint64_t));
        stream.read(reinterpret_cast<char *>(&figureMaps[i]), sizeof(uint64_t));

        uint8_t resultsSize;
        stream.read(reinterpret_cast<char *>(&resultsSize), sizeof(uint8_t));

        // correct moves read
        correctResults[i].resize(resultsSize);
        for (size_t j = 0; j < resultsSize; ++j)
            stream.read(reinterpret_cast<char *>(&correctResults[i][j]), sizeof(uint64_t));

        if (!stream)
            throw std::runtime_error(
                std::format("[ ERROR ] Encountered ill-formed record inside the test. Test no {} (0-based).", i)
            );
    }

    return {recordCount, std::move(fullMaps), std::move(figureMaps), std::move(correctResults)};
}
