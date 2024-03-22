//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/TestsAndDebugging/SearchPerfTester.h"

#include <chrono>
#include <format>
#include <fstream>

#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/Interface/FenTranslator.h"
#include "../include/Interface/Logger.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/TestsAndDebugging/CsvOperator.h"

bool SearchPerfTester::PerformSearchPerfTest(const std::string& inputTestPath, const std::string& output,
                                             stack<Move, DefaultStackSize>& stack)
{
    // reading csv file
    auto tests = CsvOperator::ReadPosDepthCsv(inputTestPath.empty() ? defPath : inputTestPath);
    if (tests.empty())
        return false;

    std::vector<std::tuple<std::string, int, double>> results{};
    double sumTime{};

    for (const auto& [testCase, dep] : tests)
    {
        const double result = _performTestCase(testCase, dep, stack);
        sumTime += result;

        results.emplace_back(testCase, dep, result);

        GlobalLogger.StartLogging() << std::format(
            "Performed test on position with depth {}:\n\t{}\nAcquired results: {}ms\n", dep, testCase, result);
    }

    results.emplace_back("Average results based on test count:", tests.size(), sumTime / tests.size());
    GlobalLogger.StartLogging() << std::format("Final average results: {}ms\n", sumTime / tests.size());

    _saveResultsToCsv(output, results);

    return true;
}

double SearchPerfTester::_performTestCase(const std::string& testCase, const int depth,
                                          stack<Move, DefaultStackSize>& stack)
{
    Board bd;
    FenTranslator::Translate(testCase, bd);
    BestMoveSearch searcher(bd, stack, 1);

    const auto tStart = std::chrono::steady_clock::now();
    [[maybe_unused]] PackedMove mv{};
    if (depth > 0)
        searcher.IterativeDeepening(&mv, depth);
    else
    {
        const int eval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.movColor);
        GlobalLogger.StartLogging() << std::format("[ INFO ] Evaluation result: {} on position:\n\t{}\n", eval, testCase);
    }
    const auto tStop = std::chrono::steady_clock::now();

    // cleaning up after test
    TTable.ClearTable();

    return (double)std::chrono::duration_cast<std::chrono::milliseconds>(tStop - tStart).count();
}

void SearchPerfTester::_saveResultsToCsv(const std::string& output,
                                         const std::vector<std::tuple<std::string, int, double>>& results)
{
    std::ofstream csvWrite(output.empty() ? defSavePath : output);

    csvWrite << "Fen position, depth, result\n";
    for (const auto& [pos, dep, res] : results)
        csvWrite << std::format("{}, {}, {}\n", pos, dep, res);
}
