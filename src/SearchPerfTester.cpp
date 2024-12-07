//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/TestsAndDebugging/SearchPerfTester.h"

#include <format>
#include <fstream>

#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/Interface/FenTranslator.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/Search/TranspositionTable.h"
#include "../include/TestsAndDebugging/CsvOperator.h"
#include "../include/ThreadManagement/GameTimeManager.h"

bool SearchPerfTester::PerformSearchPerfTest(
    const std::string &inputTestPath, const std::string &output, Stack<Move, DEFAULT_STACK_SIZE> &stack
)
{
    // reading csv file
    auto tests = CsvOperator::ReadPosDepthCsv(inputTestPath.empty() ? defPath : inputTestPath);
    if (tests.empty())
        return false;

    GameTimeManager::ShouldStop = false;

    std::vector<std::tuple<std::string, int, double>> results{};
    std::vector<double> indexes{};
    double sumTime{};

    for (const auto &[testCase, dep] : tests)
    {
        const auto [ result, avgIndex ] = _performTestCase(testCase, dep, stack);
        indexes.push_back(avgIndex);
        sumTime += result;

        results.emplace_back(testCase, dep, result);

        GlobalLogger.LogStream << std::format(
            "Performed test on position with depth {}:\n\t{}\nAcquired results: {}ms\n", dep, testCase, result
        ) << std::endl;
    }

    results.emplace_back("Average results based on test count:", tests.size(), sumTime / tests.size());
    GlobalLogger.LogStream << std::format("Final average results: {}ms\n", sumTime / tests.size()) << std::endl;
    GlobalLogger.LogStream << "Final average indexes:\n";
    for (const auto index : indexes)
        GlobalLogger.LogStream << std::format("\t{}\n", index);
    GlobalLogger.LogStream << std::endl;

    _saveResultsToCsv(output, results);

    return true;
}

std::tuple<double, double>
SearchPerfTester::_performTestCase(const std::string &testCase, const int depth, Stack<Move, DEFAULT_STACK_SIZE> &stack)
{
    Board bd;
    FenTranslator::Translate(testCase, bd);
    auto searcher = std::make_shared<BestMoveSearch>(bd, stack);

    const auto tStart = std::chrono::steady_clock::now();
    if (depth > 0)
        searcher->IterativeDeepening(nullptr, nullptr, depth);
    else
    {
        const int eval = BoardEvaluator::DefaultFullEvalFunction(bd, bd.MovingColor);
        GlobalLogger.LogStream << std::format("[ INFO ] Evaluation result: {} on position:\n\t{}\n", eval, testCase);
    }
    const auto tStop = std::chrono::steady_clock::now();

    // cleaning up after test
    TTable.ClearTable();

    searcher->CollectedData.FinishCollecting<false>();
    return {(double)std::chrono::duration_cast<std::chrono::milliseconds>(tStop - tStart).count(), searcher->CollectedData.averageCuttOffIndex};
}

void SearchPerfTester::_saveResultsToCsv(
    const std::string &output, const std::vector<std::tuple<std::string, int, double>> &results
)
{
    std::ofstream csvWrite(output.empty() ? defSavePath : output);

    csvWrite << "Fen position, depth, result\n";
    for (const auto &[pos, dep, res] : results) csvWrite << std::format("{}, {}, {}\n", pos, dep, res);
}
