//
// Created by Jlisowskyy on 2/12/24.
//

#ifndef MOVEGENERATIONTESTS_H
#define MOVEGENERATIONTESTS_H

#ifdef __unix__

#include <string>
#include <map>
#include <utility>

class MoveGenerationTester {
    // ------------------------------
    // Class creation
    // ------------------------------
public:
    MoveGenerationTester() = default;
    MoveGenerationTester(std::string comparedEnginePath): _enginePath(std::move(comparedEnginePath)) {}
    ~MoveGenerationTester() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // returns [encoded invalid move, depth of move] - where depth could be equal to:
    //  - passed depth parameter when final count on leaves differs,
    //  - 0 - when error occured in root
    //  - "-1" when no error occured
    std::pair<std::string, int> PerformSingleShallowTest(const std::string& fenPosition, int depth,
        const std::vector<std::string>& moves, bool writeOnOut = false) const;

    void PerformDeepTest(const std::string& fenPosition, int depth, const std::vector<std::string>& moves) const;

    void PerformFullTest(const std::string& fenPosition, int depth, const std::vector<std::string>& moves) const;

    // ------------------------------
    // Private class methods
    // ------------------------------
private:
    static constexpr size_t WritePipe = 1;
    static constexpr size_t ReadPipe = 0;
    static constexpr size_t buffSize = 4096;

    void _deepTestRecu(const std::string& fenPosition, int depth, std::vector<std::string>& moves, std::string& chainOut) const;

    [[nodiscard]] std::map<std::string, uint64_t> _generateCorrectMoveCounts(const std::string& fenPosition,
                                                                             int depth, const std::vector<std::string>& moves) const;

    static size_t _chessSubstrEnd(const std::string& str);
    static void _processLine(std::map<std::string, uint64_t>& out, const std::string& line);
    static std::map<std::string, uint64_t> _getCorrectMovesMap(int readFileDesc);
    static void _startUpPerft(const std::string& fenPosition, int depth, const std::vector<std::string>& moves, int writeFileDesc);
    void _spawnEngine(const int*  inPipeFileDesc, const int* outPipeFileDesc) const;

    // ------------------------------
    // Class fields
    // ------------------------------
public:

    static constexpr const char* DefaultPath = "/home/Jlisowskyy/Repos/ChessEngine/Tests/correctnesGen/stockfish";
private:
    const std::string _enginePath = DefaultPath;
};

#else // __unix__

#include "../Interface/Logger.h"

struct MoveGenerationTester {
    MoveGenerationTester( [[maybe_unused]] const std::string& unused) {}

    void PerformSingleTest([[maybe_unused]]const std::string& fenPosition, [[maybe_unused]]const int depth) const
    {
        GlobalLogger.StartErrLogging() << "[ ERROR ] Tests supported only under unix compatible platforms!\n";
    }

    void PerformDeepTest([[maybe_unused]] const std::string& fenPosition, [[maybe_unused]] int depth,
        [[maybe_unused]] const std::vector<std::string>& moves) const
    {
        PerformSingleTest(fenPosition, depth);
    }
};

#endif // __unix__

#endif //MOVEGENERATIONTESTS_H
