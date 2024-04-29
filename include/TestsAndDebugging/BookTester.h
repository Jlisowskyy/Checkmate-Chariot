//
// Created by Jlisowskyy on 2/26/24.
//

#ifndef BOOKTESTER_H
#define BOOKTESTER_H

#include <chrono>
#include <string>

#include "../OpeningBook/OpeningBook.h"

/*          IMPORTANT NOTES
 *  Refer to OpeningBook.h header to get information
 *   about the expected file format.
 */

struct BookTester
{
    // ------------------------------
    // Class creation
    // ------------------------------

    BookTester()  = delete;
    ~BookTester() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    static void
    PerformRandomAccessTest(const OpeningBook &book, const std::string &textBookPath, uint64_t testPositionCount);

    // ------------------------------
    // private methods
    // ------------------------------

    private:
    static void _testSequence(const OpeningBook &book, const std::vector<std::string> &moves, const std::string &buff);

    // simply generates vector of indices, indicating which lines will be tested
    static std::vector<uint64_t> _generateTestRecordsNums(uint64_t testPositionCount, uint64_t lineCount);

    // generate random offset from range [1, averageOffset]
    static uint64_t _getOffset(uint64_t averageOffset);
};

#endif // BOOKTESTER_H
