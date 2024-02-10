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
 *      - uint64_t - full map - 8 bytes
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
    static void PerformTest(const std::string& filename, const MapT& map) noexcept(false) {
        auto [recordCount, fullMaps, figureMaps, correctMoves] = _readTestFile(filename);
        uint64_t errorCount{};
        uint64_t moveCount{};
        uint64_t lastErrorMove{};
        uint64_t lastErrorMoveCorrectOne{};
        uint64_t lastErrorMap{};
        uint64_t lastFigPos{};

        for (size_t i = 0; i < recordCount; ++i) {
            const uint64_t fullMap = fullMaps[i];
            uint64_t figureMap = figureMaps[i];

            int figNum{};
            while(figureMap != 0) {
                const int msbPos = ExtractMsbPos(figureMap);
                ++moveCount;

                if (const uint64_t move = map.GetMoves(msbPos, fullMap); move != correctMoves[i][figNum]) {
                    ++errorCount;
                    lastErrorMove = move;
                    lastErrorMoveCorrectOne = correctMoves[i][figNum];
                    lastErrorMap = fullMap;
                    lastFigPos = maxMsbPossible >> msbPos;

                }

                ++figNum;
                figureMap ^= maxMsbPossible >> msbPos;
            }
        }

        // Test summary
        std::cout << "Test finished!!\n" << std::format("Total errors encoutered: {}\n", errorCount);
        std::cout << std::format("Processed records: {}\nTotally checked: {} moves\n", recordCount, moveCount);
        if (errorCount) {
            std::cout << std::format("________Last error move on field {}:\n\n", fieldStrMap.at((Field)lastFigPos));
            DisplayMask(lastErrorMove);
            std::cout << "________Correct move:\n\n";
            DisplayMask(lastErrorMoveCorrectOne);
            std::cout << "________On map:\n\n";
            DisplayMask(lastErrorMap);
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
        stream.read(reinterpret_cast<char *>(&recordCount), sizeof(uint64_t));


        fullMaps.resize(recordCount);
        figureMaps.resize(recordCount);
        correctResults.resize(recordCount);

        // records read
        for(size_t i = 0; i < recordCount; ++i) {
            stream.read(reinterpret_cast<char *>(&fullMaps[i]), sizeof(uint64_t));
            stream.read(reinterpret_cast<char *>(&figureMaps[i]), sizeof(uint64_t));

            uint8_t resultsSize;
            stream.read(reinterpret_cast<char *>(&resultsSize), sizeof(uint8_t));

            // correct moves read
            correctResults[i].resize(resultsSize);
            for (size_t j = 0; j < resultsSize; ++j)
                stream.read(reinterpret_cast<char *>(&correctResults[i][j]), sizeof(uint64_t));

            if (!stream)
                throw std::runtime_error(std::format("[ ERROR ] Encountered ill-formed record inside the test. Test no {} (0-based).", i));
        }

        return { recordCount, std::move(fullMaps), std::move(figureMaps), std::move(correctResults) };
    }
};

#endif //MAPCORRECTNESSTEST_H
