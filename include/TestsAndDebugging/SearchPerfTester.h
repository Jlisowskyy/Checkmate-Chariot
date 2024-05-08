//
// Created by Jlisowskyy on 3/3/24.
//

#ifndef SEARCHPERFTESTER_H
#define SEARCHPERFTESTER_H

#include <string>
#include <vector>

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"
#include "../ThreadManagement/Stack.h"

struct SearchPerfTester
{
    // ------------------------------
    // Class creation
    // ------------------------------

    SearchPerfTester()  = default;
    ~SearchPerfTester() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    static bool PerformSearchPerfTest(
        const std::string &inputTestPath, const std::string &output, Stack<Move, DefaultStackSize> &stack
    );

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    [[nodiscard]] static double
    _performTestCase(const std::string &testCase, int depth, Stack<Move, DefaultStackSize> &stack);
    static void
    _saveResultsToCsv(const std::string &output, const std::vector<std::tuple<std::string, int, double>> &results);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr auto defPath     = "Tests/searchTests.csv";
    static constexpr auto defSavePath = "Tests/search_perf_out.csv";
};

#endif // SEARCHPERFTESTER_H
