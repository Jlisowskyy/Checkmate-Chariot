//
// Created by Jlisowskyy on 2/9/24.
//

#ifndef MAPCORRECTNESSTEST_H
#define MAPCORRECTNESSTEST_H

#include <string>
#include <fstream>
#include <format>
#include <iostream>
#include <vector>

/*              IMPORTANT NOTES
 *  All files containing maps used to test must follow this scheme:
 *  - uint64_t - containing number of records - 8 bytes
 *  - records should be saved in such manner that:
 *      - uint64_t - full map without rooks or bishops - 8 bytes
 *      - uint64_t - map containing only rooks or bishops - 8 bytes
 *      - uint8_t - contains number of following results, all should be sorted from highest board index to lowest - 1 byte
 *      - uint64_t - move results for each figure from highest board index to lowest - 8 bytes each
 */

// Class used to perform tests on moves maps, based on maps stored inside files
class MapCorrectnessTester {
    // ------------------------------
    // inner class types
    // ------------------------------

    using RecordsPack = std::tuple<uint64_t, std::vector<uint64_t>, std::vector<uint64_t>, std::vector<std::vector<uint64_t>>>;
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

    template<class MapT>
    static void PerformanceTest(const std::string& filename, const MapT& map) noexcept(false) {
        auto [recordCount, fullMaps, figureMaps, correctMoves] = _readTestFile(filename);
        uint64_t errorCount{};
        uint64_t lastErrorMove{};
        uint64_t lastErrorMoveCorrectOne{};

        for (size_t i = 0; i << recordCount; ++i) {
            const uint64_t fullMap = fullMaps[i];
            uint64_t figureMap = figureMaps[i];

            int figNum{};
            while(figureMap != 0) {
                const int msbPos = ExtractMsbPos(figureMap);

                if (const uint64_t move = map.GetMoves(msbPos, fullMap); move != correctMoves[i][figNum]) {
                    ++errorCount;
                    lastErrorMove = move;
                    lastErrorMoveCorrectOne = correctMoves[i][figNum];
                }

                ++figNum;
                figureMap ^=  maxMsbPossible >> msbPos;
            }
        }

        // Test summary
        std::cout << "Test finished!!\n" << std::format("Total errors encoutered: {}\n", errorCount);
        if (errorCount) {
            std::cout << "Last error move:\n";
            DisplayMask(lastErrorMove);
            std::cout << "Correct move:\n";
            DisplayMask(lastErrorMoveCorrectOne);
        }
    }

    // ------------------------------
    // Private class methods
    // ------------------------------
private:

    static RecordsPack _readTestFile(std::string filename) {
        std::fstream stream(filename, std::fstream::binary | std::fstream::in);

        if (!stream)
            throw std::runtime_error(std::format("[ ERROR ] Failed when opening file: {}!\n", filename));

        uint64_t recordCount{};
        std::vector<uint64_t> fullMaps{};
        std::vector<uint64_t> figureMaps{};
        std::vector<std::vector<uint64_t>> correctResults{};

        // file size read
        stream >> recordCount;

        fullMaps.resize(recordCount);
        figureMaps.resize(recordCount);
        correctResults.resize(recordCount);

        // records read
        for(size_t i = 0; i < recordCount; ++i) {
            stream >> fullMaps[i];
            stream >> figureMaps[i];

            uint8_t resultsSize;
            stream >> resultsSize;

            // correct moves read
            correctResults[i].resize(resultsSize);
            for (size_t j = 0; j < resultsSize; ++j)
                stream >> correctResults[i][j];

            if (!stream)
                throw std::runtime_error(std::format("[ ERROR ] Encountered ill-formed record inside the test. Test no {} (0-based).", i));
        }

        return { recordCount, std::move(fullMaps), std::move(figureMaps), std::move(correctResults) };
    }
};

#endif //MAPCORRECTNESSTEST_H
