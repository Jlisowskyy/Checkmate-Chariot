//
// Created by Jlisowskyy on 2/26/24.
//

#include "../include/TestsAndDebugging/BookTester.h"

#include <algorithm>
#include <format>
#include <fstream>
#include <random>

#include "../include/Interface/Logger.h"
#include "../include/ParseTools.h"

void BookTester::PerformRandomAccessTest(
    const OpeningBook &book, const std::string &textBookPath, const uint64_t testPositionCount
)
{
    std::fstream stream(textBookPath, std::ios::in);

    if (!stream)
        throw std::runtime_error("[ ERROR ] Not able to open the passed book directory!");

    const uint64_t lineCount               = ParseTools::GetLineCountFromFile(stream);
    std::vector<uint64_t> testRecordsLines = _generateTestRecordsNums(testPositionCount, lineCount);

    std::string lineBuff{};
    // retrieving wanted records from file
    for (uint64_t lineNum{}; auto line : testRecordsLines)
    {
        // skipping unwanted lines
        while (lineNum++ != line) std::getline(stream, lineBuff);

        // getting wanted line
        std::getline(stream, lineBuff);

        // extracting moves
        auto moves = ParseTools::Split(lineBuff);

        _testSequence(book, moves, lineBuff);
    }
}

void BookTester::_testSequence(const OpeningBook &book, const std::vector<std::string> &moves, const std::string &buff)
{
    for (size_t i = 1; i < moves.size(); ++i)
    {
        const std::vector vect(moves.begin(), moves.begin() + 1);

        if (book.GetRandomNextMove(vect).empty())
            GlobalLogger.StartLogging(
            ) << std::format("[ ERROR ] Internal consistency of structure was not met!\n\tOn moves: {}\n", buff);
    }
}

std::vector<uint64_t> BookTester::_generateTestRecordsNums(const uint64_t testPositionCount, const uint64_t lineCount)
{
    const uint64_t averageOffset = static_cast<uint64_t>(
        std::max(1.0, std::floor(static_cast<double>(lineCount) / static_cast<double>(testPositionCount)))
    );

    std::vector<uint64_t> positions{};
    for (uint64_t pos = _getOffset(averageOffset); pos < lineCount; pos += _getOffset(averageOffset))
        positions.push_back(pos);

    return positions;
}

uint64_t BookTester::_getOffset(const uint64_t averageOffset)
{
    static std::mt19937_64 randEng(std::chrono::steady_clock::now().time_since_epoch().count());
    return 1 + randEng() % averageOffset;
}
