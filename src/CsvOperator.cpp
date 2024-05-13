//
// Created by Jlisowskyy on 3/3/24.
//

#include "../include/TestsAndDebugging/CsvOperator.h"

#include <format>
#include <fstream>

#include "../include/Interface/Logger.h"
#include "../include/ParseTools.h"

std::vector<std::pair<std::string, int>> CsvOperator::ReadPosDepthCsv(std::ifstream &stream)
{
    std::vector<std::pair<std::string, int>> records{};

    for (std::string buffer{}; std::getline(stream, buffer);)
    {
        std::string innerBuf{};
        buffer = ParseTools::GetTrimmed(buffer);
        if (buffer.empty())
            continue;

        size_t pos = ParseTools::ExtractNextWord<
            [](const int x) -> int
            {
                return x == ',';
            }>(buffer, innerBuf, 0);
        if (pos == 0)
            throw std::runtime_error("missing comma: expected two values fen position and depth");

        innerBuf.pop_back();
        records.emplace_back(innerBuf, 0);

        pos = ParseTools::ExtractNextWord(buffer, innerBuf, pos + 1);
        if (pos == 0)
            throw std::runtime_error("missing second argument: expected two values fen position and depth");

        records.back().second = static_cast<int>(std::stoll(innerBuf));
    }

    return records;
}

std::vector<std::pair<std::string, int>> CsvOperator::ReadPosDepthCsv(const std::string &fileName)
{
    std::ifstream csvRead(fileName);

    // reading tests from file
    std::vector<std::pair<std::string, int>> tests{};
    try
    {
        tests = ReadPosDepthCsv(csvRead);
    }
    catch (const std::exception &exc)
    {
        GlobalLogger.LogStream << std::format("[ ERROR ] Error occured:\n{}\n", exc.what());
        return {};
    }

    return tests;
}
