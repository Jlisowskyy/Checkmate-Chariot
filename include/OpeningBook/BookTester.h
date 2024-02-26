//
// Created by Jlisowskyy on 2/26/24.
//

#ifndef BOOKTESTER_H
#define BOOKTESTER_H

#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <random>
#include <format>

#include "OpeningBook.h"
#include "../ParseTools.h"

/*          IMPORTANT NOTES
 *  Refer to OpenningBook.h header to get information
 *  about expected file format.
 */

struct BookTester
{
    // ------------------------------
    // Class creation
    // ------------------------------

    BookTester() = delete;
    ~BookTester() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    static void PerformRandomAccessTest(const OpenningBook& book, const std::string& textBookPath, const uint64_t testPositionCount)
    {
        std::fstream stream(textBookPath, std::ios::in);

        if (!stream)
            throw std::runtime_error("[ ERROR ] Not able to open the passed book directory!");

        const uint64_t lineCount = ParseTools::GetLineCountFromFile(stream);
        std::vector<uint64_t> testRecordsLines = _generateTestRecordsNums(testPositionCount, lineCount);

        std::string lineBuff{};
        // retreiving wanted records from file
        for (uint64_t lineNum{}; auto line: testRecordsLines)
        {
            // skipping unwanted lines
            while (lineNum++ != line) std::getline(stream, lineBuff);

            // getting wanted line
            std::getline(stream, lineBuff);

            // extracting moves
            auto moves = ParseTools::Split(lineBuff);
        }
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    static void _testSequence(std::vector<std::string>& moves){

    }

    // simply generates vector of indices, indicating which lines will be tested
    static std::vector<uint64_t> _generateTestRecordsNums(const uint64_t testPositionCount, const uint64_t lineCount)
    {
        uint64_t averageOffset = static_cast<uint64_t>(
            std::max(1.0, std::floor(static_cast<double>(lineCount) / static_cast<double>(testPositionCount))));

        std::vector<uint64_t> positions{};
        for(uint64_t pos = _getOffset(averageOffset); pos < lineCount; pos += _getOffset(averageOffset))
            positions.push_back(pos);

        return positions;
    }

    // generate random offset from range [1, averageOffset]
    static uint64_t _getOffset(const uint64_t averageOffset)
    {
        static std::mt19937_64 randEng(std::chrono::steady_clock::now().time_since_epoch().count());
        return 1 + randEng() % averageOffset;
    }
};

#endif //BOOKTESTER_H
