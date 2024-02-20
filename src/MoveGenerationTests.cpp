//
// Created by Jlisowskyy on 2/16/24.
//

#include <stack>
#include <unistd.h>
#include <sys/prctl.h>
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <format>

#include "../include/MoveGeneration/ChessMechanics.h"
#include "../include/ParseTools.h"
#include "../include/TestsAndDebugging/MoveGenerationTests.h"
#include "../include/Engine.h"
#include "../include/Interface/Logger.h"

std::pair<std::string, int> MoveGenerationTester::PerformSingleShallowTest(const std::string& fenPosition,
    const int depth, const std::vector<std::string>& moves, const bool writeOnOut) const {
    Engine eng{};
    eng.SetFenPosition(fenPosition);
    eng.ApplyMoves(moves);

    const auto externalEngineMoves = _generateCorrectMoveCounts(fenPosition, depth, moves);
    const auto internalEngineMoves = eng.GetPerft(depth);

    for (const auto& [move, count] : externalEngineMoves) {

        // first case : move not detected
        if (!internalEngineMoves.contains(move)) {
            if (writeOnOut)
                GlobalLogger.StartLogging() << std::format("[ ERROR ] Internal Engine didnt detect move: {} on first depth!\n", move);
            return { move, 0 };
        }

        // second case : wrong number of moves in deeper layers
        if (internalEngineMoves.at(move) != count) {

            if (writeOnOut)
                GlobalLogger.StartLogging() << std::format("[ ERROR ] Engine detected wrong number on move: {}\nCorrect one: {}\nBut returned: {}\n",
                                                           move, count, internalEngineMoves.at(move));

            return { move, depth };
        }
    }

    // third case : additional moves were generated
    if (internalEngineMoves.size() > externalEngineMoves.size()) {
        for (const auto& [move, _] : internalEngineMoves)
            if (!externalEngineMoves.contains(move)) {

                if (writeOnOut)
                    GlobalLogger.StartLogging() << std::format("[ ERROR ] Engine made additional move: {}!\n", move);

                return { move, 0 };

            }
    }

    if (writeOnOut)
        GlobalLogger.StartLogging() << "[  OK  ] All moves were generated correctly!\n";

    return { "", -1 };
}

void MoveGenerationTester::PerformDeepTest(const std::string& fenPosition,
    const int depth, const std::vector<std::string>& moves) const
{
    std::string invalidMoveChain;
    std::vector<std::string> innerMoves = moves;
    _deepTestRecu(fenPosition, depth, innerMoves, invalidMoveChain);

    if (invalidMoveChain.empty())
        GlobalLogger.StartLogging() << "[  OK  ] No errors occured!\n";
    else {
        GlobalLogger.StartLogging() << std::format("[ ERROR ] Found invalind moves chain:\n\t{}\n",
                                                  invalidMoveChain + " NULL");

        std::string moveString{};
        for(const auto& move: innerMoves) moveString += move + ' ';

        GlobalLogger.StartLogging() << std::format("\tReady pos command:\n\tposition startpos moves {}\n", moveString);
    }
}

void MoveGenerationTester::PerformFullTest(const std::string& fenPosition, const int depth,
    const std::vector<std::string>& moves) const
{
    bool shouldEnd = false;
    Engine eng{};
    eng.SetFenPosition(fenPosition);
    eng.ApplyMoves(moves);

    auto board = eng.GetUnderlyingBoardCopy();
    ChessMechanics mech{board};
    mech.IterativeBoardTraversal(
        [&](const Board& bd) {
            if (shouldEnd) return;

            const auto fenEncoding = FenTranslator::Translate(bd);
            const auto [move, dep] = PerformSingleShallowTest(fenEncoding, 1, std::vector<std::string>{});

            if (dep != -1) {
                shouldEnd = true;
                GlobalLogger.StartLogging() << std::format("[ ERROR ] Invalid position found: {}\n And move: {}\n", fenEncoding, move);
            }
        },
        depth-1
    );

    if (shouldEnd == false) {
        GlobalLogger.StartLogging() << "[  OK  ] No invalid positions found!\n";
    }
}

void MoveGenerationTester::_deepTestRecu(const std::string& fenPosition, const int depth,
                                         std::vector<std::string>& moves, std::string& chainOut) const
{
    if (depth == 0) return;

    // Performing calculation on actual layer
    auto [move, errDep] = PerformSingleShallowTest(fenPosition, depth, moves, true);

    // skipping deeper search when moves does not differ
    if (errDep == -1) return;

    // otherwise adding move to the chain
    chainOut += move + " ==> ";
    moves.push_back(move);

    // if error occured on actual layer stop adding moves to the chain
    if (errDep == 0) return;

    // otherwise add other moves
    _deepTestRecu(fenPosition, depth-1, moves, chainOut);
}

std::map<std::string, uint64_t> MoveGenerationTester::_generateCorrectMoveCounts(const std::string& fenPosition,
                                                                                 const int depth, const std::vector<std::string>& moves) const {
    // preparing pipes. 1 - input, 0 - output
    int inPipeFileDesc[2];
    int outPipeFileDesc[2];

    if (pipe(inPipeFileDesc)) {
        perror("inPipeFileDesc init failed!");
        exit(EXIT_FAILURE);
    }
    if (pipe(outPipeFileDesc)) {
        perror("outPipeFileDesc init failed!");
        exit(EXIT_FAILURE);
    }

    // fork branching
    const pid_t proc = fork();
    if (proc == 0) {
        _spawnEngine(inPipeFileDesc, outPipeFileDesc);
        exit(EXIT_FAILURE);
    }

    // plumbing not used pipe sides
    close(outPipeFileDesc[ReadPipe]);
    close(inPipeFileDesc[WritePipe]);

    _startUpPerft(fenPosition, depth, moves, outPipeFileDesc[WritePipe]);
    auto moveMap = _getCorrectMovesMap(inPipeFileDesc[ReadPipe]);

    waitpid(proc, nullptr, 0);
    return moveMap;
}

size_t MoveGenerationTester::_chessSubstrEnd(const std::string& str) {
    auto isNotAlpCord = [](const int c) {
        return c > 'H' || c < 'A';
    };

    auto isNotNumCord = [](const int c) {
        return c > '8' || c < '1';
    };

    if (str.length() < 4) return 0;

    if (isNotAlpCord(std::toupper(str[0]))
        || isNotNumCord(str[1])
        || isNotAlpCord(std::toupper(str[2]))
        || isNotNumCord(str[3]))
        return 0;

    static constexpr auto upgradeFigs = "RNBQ";
    if (str.length() > 4) {
        for (size_t i = 0; i < 4; ++i)
            if (upgradeFigs[i] == std::toupper(str[4])) return 5;
    }

    return 4;
}

void MoveGenerationTester::_processLine(std::map<std::string, uint64_t>& out, const std::string& line) {
    // string preparation
    const auto trimmed = ParseTools::GetTrimmed(line);

    // preparing result storing variables
    std::string positionEncoding{};
    lli movesCount{};

    // preparing helping variables
    size_t pos = 0;
    std::string wordBuffer{};

    // extracting position encoding
    while ((pos = ParseTools::ExtractNextWord(trimmed, wordBuffer, pos)) != 0) {
        if (_chessSubstrEnd(wordBuffer) > 0) {
            positionEncoding = wordBuffer.substr(0, _chessSubstrEnd(wordBuffer));
            wordBuffer.clear();
            break;
        }
        wordBuffer.clear();
    }

    if (pos == 0) return;

    while ((pos = ParseTools::ExtractNextWord(trimmed, wordBuffer, pos)) != 0) {
        const lli moves = ParseTools::ParseTolli(wordBuffer);

        if (errno == 0) {
            movesCount = moves;
            wordBuffer.clear();
            break;
        }
        wordBuffer.clear();
    }

    if (pos == 0) return;

    if (out.contains(positionEncoding))
        throw std::runtime_error(std::format("[ WARNING ] Given engine generated multiple results for same position:{}\nWith counts: {}, {}\n",
                                             positionEncoding, movesCount, out[positionEncoding]));

    out[positionEncoding] = movesCount;
}

std::map<std::string, uint64_t> MoveGenerationTester::_getCorrectMovesMap(const int readFileDesc) {
    // buffer used to read answers
    char buf[buffSize];
    std::map<std::string, uint64_t> moveMap {};
    std::stack<std::string> lines{};

    ssize_t size;
    std::string lineBuff{};
    while((size = read(readFileDesc, buf, buffSize)) > 0){
        ssize_t pos{};

        while((pos = static_cast<ssize_t>(ParseTools::ExtractNextLine(pos, size, buf, lineBuff))) < size) {
            _processLine(moveMap, lineBuff);
            lineBuff.clear();
        }

        if (buf[size-1] == '\n' || lineBuff.size() > buffSize) {
            _processLine(moveMap, lineBuff);
            lineBuff.clear();
        }
    }

    return moveMap;
}

void MoveGenerationTester::_startUpPerft(const std::string& fenPosition, const int depth, const std::vector<std::string>& moves, const int writeFileDesc) {
    static constexpr auto quitCommand = "quit\n";
    const std::string fenCommand = std::format("position fen {}\n", fenPosition);
    const std::string perftCommand = std::format("go perft {}\n", depth);

    write(writeFileDesc, fenCommand.c_str(), fenCommand.size());

    // applying moves to the external engine
    if (!moves.empty()) {
        std::string moveString = "position startpos moves ";
        for (const auto& move: moves)
            moveString += move + ' ';

        moveString += '\n';
        write(writeFileDesc, moveString.c_str(), moveString.size());
    }

    write(writeFileDesc, perftCommand.c_str(), perftCommand.size());
    write(writeFileDesc, quitCommand, strlen(quitCommand));
}

void MoveGenerationTester::_spawnEngine(const int* const inPipeFileDesc, const int* const outPipeFileDesc) const {
    // connecting with pipes
    if (dup2(outPipeFileDesc[ReadPipe], STDIN_FILENO) == -1) {
        perror("Not able to comunicate with other engine!");
        exit(EXIT_FAILURE);
    }

    if (dup2(inPipeFileDesc[WritePipe], STDOUT_FILENO) == -1){
        perror("Not able to comunicate with other engine!");
        exit(EXIT_FAILURE);
    };

    if (dup2(inPipeFileDesc[WritePipe], STDERR_FILENO) == -1){
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
