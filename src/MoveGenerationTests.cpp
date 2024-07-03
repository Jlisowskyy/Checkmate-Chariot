//
// Created by Jlisowskyy on 2/16/24.
//

#ifdef __unix__

#include <cstring>
#include <format>
#include <fstream>
#include <stack>
#include <string>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/Engine.h"
#include "../include/ParseTools.h"
#include "../include/TestsAndDebugging/CsvOperator.h"
#include "../include/TestsAndDebugging/MoveGenerationTests.h"

std::pair<std::string, int> MoveGenerationTester::PerformSingleShallowTest(
    const std::string &fenPosition, const int depth, const std::vector<std::string> &moves, const bool writeOnOut
) const
{
    Engine eng{};
    eng.SetFenPosition(fenPosition);
    eng.ApplyMoves(moves);

    const auto externalEngineMoves = _generateCorrectMoveCounts(fenPosition, depth, moves);
    const auto internalEngineMoves = eng.GetPerft(depth);

    for (const auto &[move, count] : externalEngineMoves)
    {
        // first case : move not detected
        if (!internalEngineMoves.contains(move))
        {
            if (writeOnOut)
                GlobalLogger.LogStream << std::format(
                    "[ ERROR ] Internal Engine didnt detect move: {} on first depth!", move
                ) << std::endl;
            return {move, 0};
        }

        // second case : wrong number of moves in deeper layers
        if (internalEngineMoves.at(move) != count)
        {
            if (writeOnOut)
                GlobalLogger.LogStream << std::format(
                    "[ ERROR ] Engine detected wrong number on move: {}\nCorrect one: {}\nBut returned: {}", move,
                    count, internalEngineMoves.at(move)
                ) << std::endl;

            return {move, depth};
        }
    }

    // third case : additional moves were generated
    if (internalEngineMoves.size() > externalEngineMoves.size())
    {
        for (const auto &[move, _] : internalEngineMoves)
            if (!externalEngineMoves.contains(move))
            {
                if (writeOnOut)
                    GlobalLogger.LogStream << std::format("[ ERROR ] Engine made additional move: {}!", move) << std::endl;

                return {move, 0};
            }
    }

    if (writeOnOut)
        GlobalLogger.LogStream << "[  OK  ] All moves were generated correctly!" << std::endl;

    return {"", -1};
}

void MoveGenerationTester::PerformDeepTest(
    const std::string &fenPosition, const int depth, const std::vector<std::string> &moves
) const
{
    std::string invalidMoveChain;
    std::vector<std::string> innerMoves = moves;
    _deepTestRecu(fenPosition, depth, innerMoves, invalidMoveChain);

    if (invalidMoveChain.empty())
        GlobalLogger.LogStream << "[  OK  ] No errors occured!" << std::endl;
    else
    {
        GlobalLogger.LogStream << std::format(
            "[ ERROR ] Found invalind moves chain:\n\t{}", invalidMoveChain + " NULL"
        ) << std::endl;

        std::string moveString{};
        for (const auto &move : innerMoves) moveString += move + ' ';

        GlobalLogger.LogStream << std::format("\tReady pos command:\n\tposition startpos moves {}", moveString) << std::endl;
    }
}

void MoveGenerationTester::PerformSeriesOfDeepTests(const std::vector<std::pair<std::string, int>> &testPositions) const
{
    for (const auto &[position, depth] : testPositions)
    {
        GlobalLogger.LogStream << std::format(
            "Starting deep debug test on position:\n\t{}\n\tWith depth: {}", position, depth
        ) << std::endl;
        PerformDeepTest(position, depth, std::vector<std::string>());
    }
}

bool MoveGenerationTester::PerformSeriesOfDeepTestFromFile(const std::string &path) const
{
    const auto tests = CsvOperator::ReadPosDepthCsv(
        path.empty()  ? DefaultTestPath
        : path == "/" ? DefaultTestPath1
                      : path
    );

    if (tests.empty())
        return false;

    PerformSeriesOfDeepTests(tests);

    return true;
}

bool MoveGenerationTester::PerformPerformanceTest(const std::string &inputTestPath, const std::string &output) const
{
    // reading csv file
    auto tests = CsvOperator::ReadPosDepthCsv(inputTestPath.empty() ? DefaultCompTestPath : inputTestPath);
    if (tests.empty())
        return false;

    // performing tests
    std::vector<std::tuple<std::string, int, double, double, double>> results{};
    double internalSum{};
    double externalSum{};

    GlobalLogger.LogStream << "All results are displayed in following manner:\n\t Internal Engine time in ms, "
                              "External Engine time in ms, ratio (external/internal)" << std::endl;

    for (const auto &[pos, dep] : tests)
    {
        double internalTime = _performEngineSpeedTest(pos, dep);
        double externalTime = _performExternalEngineSpeedTest(pos, dep);
        double ratio        = externalTime / internalTime;

        internalSum += internalTime;
        externalSum += externalTime;

        results.emplace_back(pos, dep, internalTime, externalTime, ratio);

        GlobalLogger.LogStream << std::format(
            "Performed test on position with depth {}:\n\t{}\nAcquired results: {}, {}, {}", dep, pos, internalTime,
            externalTime, ratio
        ) << std::endl;
    }

    // last summary note
    const double tCount = tests.size();
    results.emplace_back(
        "Average results based on test count:", tCount, internalSum / tCount, externalSum / tCount,
        externalSum / internalSum
    );
    GlobalLogger.LogStream << std::format(
        "Final average results: {}, {}, {}", internalSum / tCount, externalSum / tCount, externalSum / internalSum
    ) << std::endl;

    // writiing to desired csv file
    std::ofstream csvWrite(output.empty() ? DefaultSaveFile : output);
    _saveResultToCsv(results, csvWrite);

    return true;
}

void MoveGenerationTester::_saveResultToCsv(
    const std::vector<std::tuple<std::string, int, double, double, double>> &results, std::ofstream &stream
)
{
    stream << "Fen Position, depth, internal engine time, external engine time, ratio (external/internal)" << std::endl;
    for (const auto &[i1, i2, i3, i4, i5] : results)
        stream << std::format("{}, {}, {}, {}, {}", i1, i2, i3, i4, i5) << std::endl;
}

double MoveGenerationTester::_performExternalEngineSpeedTest(const std::string &fenPosition, int depth) const
{
    static constexpr double BootupDelay = 100;

    const auto t1 = std::chrono::steady_clock::now();

    const auto [communcationChannel, process] = _getExternalEngineProcess();
    _startUpPerft(fenPosition, depth, std::vector<std::string>(), communcationChannel[WritePipe]);
    waitpid(process, nullptr, 0);

    close(communcationChannel[ReadPipe]);
    close(communcationChannel[WritePipe]);

    const auto t2 = std::chrono::steady_clock::now();

    return static_cast<double>((t2 - t1).count()) * 1e-6 - BootupDelay;
}

double MoveGenerationTester::_performEngineSpeedTest(const std::string &fenPosition, const int depth)
{
    Engine eng{};
    eng.SetFenPosition(fenPosition);

    return eng.GoPerft<false>(depth);
}

void MoveGenerationTester::_deepTestRecu(
    const std::string &fenPosition, const int depth, std::vector<std::string> &moves, std::string &chainOut
) const
{
    if (depth == 0)
        return;

    // Performing calculation on actual layer
    auto [move, errDep] = PerformSingleShallowTest(fenPosition, depth, moves, true);

    // skipping deeper search when moves does not differ
    if (errDep == -1)
        return;

    // otherwise adding move to the chain
    chainOut += move + " ==> ";
    moves.push_back(move);

    // if error occured on actual layer Stop adding moves to the chain
    if (errDep == 0)
        return;

    // otherwise add other moves
    _deepTestRecu(fenPosition, depth - 1, moves, chainOut);
}

std::map<std::string, uint64_t> MoveGenerationTester::_generateCorrectMoveCounts(
    const std::string &fenPosition, const int depth, const std::vector<std::string> &moves
) const
{
    const auto [communcationChannel, process] = _getExternalEngineProcess();

    _startUpPerft(fenPosition, depth, moves, communcationChannel[WritePipe]);
    auto moveMap = _getCorrectMovesMap(communcationChannel[ReadPipe]);

    waitpid(process, nullptr, 0);

    close(communcationChannel[ReadPipe]);
    close(communcationChannel[WritePipe]);

    return moveMap;
}

size_t MoveGenerationTester::_chessSubstrEnd(const std::string &str)
{
    auto isNotAlpCord = [](const int c)
    {
        return c > 'H' || c < 'A';
    };

    auto isNotNumCord = [](const int c)
    {
        return c > '8' || c < '1';
    };

    if (str.length() < 4)
        return 0;

    if (isNotAlpCord(std::toupper(str[0])) || isNotNumCord(str[1]) || isNotAlpCord(std::toupper(str[2])) ||
        isNotNumCord(str[3]))
        return 0;

    static constexpr auto upgradeFigs = "RNBQ";
    if (str.length() > 4)
    {
        for (size_t i = 0; i < 4; ++i)
            if (upgradeFigs[i] == std::toupper(str[4]))
                return 5;
    }

    return 4;
}

void MoveGenerationTester::_processLine(std::map<std::string, uint64_t> &out, const std::string &line)
{
    // string preparation
    const auto trimmed = ParseTools::GetTrimmed(line);

    // preparing result storing variables
    std::string positionEncoding{};
    lli movesCount{};

    // preparing helping variables
    size_t pos = 0;
    std::string wordBuffer{};

    // extracting position encoding
    while ((pos = ParseTools::ExtractNextWord(trimmed, wordBuffer, pos)) != 0)
    {
        if (_chessSubstrEnd(wordBuffer) > 0)
        {
            positionEncoding = wordBuffer.substr(0, _chessSubstrEnd(wordBuffer));
            wordBuffer.clear();
            break;
        }
        wordBuffer.clear();
    }

    if (pos == 0)
        return;

    while ((pos = ParseTools::ExtractNextWord(trimmed, wordBuffer, pos)) != 0)
    {
        try
        {
            const lli moves = std::stoll(wordBuffer);
            movesCount      = moves;
            wordBuffer.clear();
            break;
        }
        catch (const std::exception &exc)
        {
            wordBuffer.clear();
        }
    }

    if (pos == 0)
        return;

    if (out.contains(positionEncoding))
        throw std::runtime_error(std::format(
            "[ WARNING ] Given engine generated multiple results for same position:{}\nWith counts: {}, {}\n",
            positionEncoding, movesCount, out[positionEncoding]
        ));

    out[positionEncoding] = movesCount;
}

std::map<std::string, uint64_t> MoveGenerationTester::_getCorrectMovesMap(const int readFileDesc)
{
    // buffer used to read answers
    char buf[buffSize];
    std::map<std::string, uint64_t> moveMap{};
    std::stack<std::string> lines{};

    signed_size_t size;
    std::string lineBuff{};
    while ((size = read(readFileDesc, buf, buffSize)) > 0)
    {
        signed_size_t pos{};

        while ((pos = static_cast<signed_size_t>(ParseTools::ExtractNextLine(pos, size, buf, lineBuff))) < size)
        {
            _processLine(moveMap, lineBuff);
            lineBuff.clear();
        }

        if (buf[size - 1] == '\n' || lineBuff.size() > buffSize)
        {
            _processLine(moveMap, lineBuff);
            lineBuff.clear();
        }
    }

    return moveMap;
}

void MoveGenerationTester::_startUpPerft(
    const std::string &fenPosition, const int depth, const std::vector<std::string> &moves, const int writeFileDesc
)
{
    static constexpr auto quitCommand = "quit\n";
    const std::string perftCommand    = std::format("go perft {}\n", depth);

    // applying moves to the external engine
    std::string fenCommand = std::format("position fen {}", fenPosition);
    if (!moves.empty())
    {
        fenCommand += " moves ";
        for (const auto &move : moves) fenCommand += move + ' ';
    }

    fenCommand += '\n';
    write(writeFileDesc, fenCommand.c_str(), fenCommand.size());

    write(writeFileDesc, perftCommand.c_str(), perftCommand.size());
    write(writeFileDesc, quitCommand, strlen(quitCommand));
}

void MoveGenerationTester::_spawnEngine(const int *const inPipeFileDesc, const int *const outPipeFileDesc) const
{
    // connecting with pipes
    if (dup2(outPipeFileDesc[ReadPipe], STDIN_FILENO) == -1)
    {
        perror("Not able to comunicate with other engine!");
        exit(EXIT_FAILURE);
    }

    if (dup2(inPipeFileDesc[WritePipe], STDOUT_FILENO) == -1)
    {
        perror("Not able to comunicate with other engine!");
        exit(EXIT_FAILURE);
    };

    if (dup2(inPipeFileDesc[WritePipe], STDERR_FILENO) == -1)
    {
        perror("Not able to comunicate with other engine!");
        exit(EXIT_FAILURE);
    };

    // enforces closing the program when parent dies
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // plumbing not used pipe sides
    close(outPipeFileDesc[WritePipe]);
    close(inPipeFileDesc[ReadPipe]);

    // executing desired enginef
    execl(_enginePath.c_str(), "stockfish", nullptr);
}

std::pair<std::array<int, 2>, pid_t> MoveGenerationTester::_getExternalEngineProcess() const
{
    // preparing pipes. 1 - input, 0 - output
    int inPipeFileDesc[2];
    int outPipeFileDesc[2];

    if (pipe(inPipeFileDesc))
    {
        perror("inPipeFileDesc init failed!");
        exit(EXIT_FAILURE);
    }
    if (pipe(outPipeFileDesc))
    {
        perror("outPipeFileDesc init failed!");
        exit(EXIT_FAILURE);
    }

    // fork branching
    const pid_t proc = fork();
    if (proc == 0)
    {
        _spawnEngine(inPipeFileDesc, outPipeFileDesc);
        exit(EXIT_FAILURE);
    }

    // plumbing not used pipe sides
    close(outPipeFileDesc[ReadPipe]);
    close(inPipeFileDesc[WritePipe]);

    return {
        {inPipeFileDesc[ReadPipe], outPipeFileDesc[WritePipe]},
        proc
    };
}

#endif // __unix__