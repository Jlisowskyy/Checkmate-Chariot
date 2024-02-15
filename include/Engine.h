//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>

#include "Interface/UCIOptions.h"
#include "EngineTypeDefs.h"
#include "Interface/FenTranslator.h"
#include "Interface/Logger.h"
#include "MoveGeneration/ChessMechanics.h"

class Engine {
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------
public:
    Engine() {}

    void Initialize() {
        board = FenTranslator::Translate(startingPosition);
    }

    // ------------------------------
    // Type interaction
    // ------------------------------

    void writeBoard() const { std::cout << board; }
    static const EngineInfo& GetEngineInfo() { return engineInfo; }
    void RestartEngine() { std::cout << "ucinewgame result! " << std::endl; }
    void StopSearch() { std::cout << "stop search resullt! " << std::endl; }
    void GoDepth(lli depth) { std::cout << "go depth resutl: " << depth << std::endl; }
    void GoMovetime(lli time) { std::cout << "go movetime resutl: " << time << std::endl; }
    void GoInfinite() { std::cout << "go infinite result! " << std::endl; }

    void GoPerft(const int depth) {
        ChessMechanics game(board);
        uint64_t totalSum{};

        const auto t1 = std::chrono::steady_clock::now();
        game.IterativeBoardTraversal(
            [&]([[maybe_unused]] const Board& board, [[maybe_unused]] const int unused3, const uint64_t oldBoard,
                const uint64_t newBoard, const MoveTypes mType)
            {
                const auto moveStr = GetShortAlgebraicMoveEncoding(board, oldBoard, newBoard, mType);

                uint64_t localSum{};
                game.IterativeBoardTraversal(
                    [&]([[maybe_unused]] Board& unused5, const int dp, [[maybe_unused]] const uint64_t unused0,
                        [[maybe_unused]] const uint64_t unused1, [[maybe_unused]] const MoveTypes unused2) {
                        if (dp == 0) ++localSum;
                    },
                    depth-1
                );

                totalSum += localSum;
                GlobalLogger.StartLogging() << std::format("{}: {}\n", moveStr, localSum);
            },
            1
        );
        const auto t2 = std::chrono::steady_clock::now();

        std::cout << std::format("Calculated moves: {} in time: {}ms\n", totalSum, (t2-t1).count()*1e-6);
    }

    void SetFenPosition(const std::string& fenStr) {
        board = FenTranslator::Translate(fenStr);
    }

    bool ApplyMoves(const std::vector<std::string>& UCIMoves) {
        std::cout << "Received moves: ";
        for(auto& move : UCIMoves)
            std::cout << move << ' ';
        std::cout << std::endl;

        return true;
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    static void _changeThreadCount(Engine& eng, lli tCount) {
        std::cout << "New thread count: " << tCount << '\n';
    }

    static void _changeDebugState(Engine& eng, std::string& nPath) {
        GlobalLogger.ChangeLogStream(nPath);
    }

    // ------------------------------
    // private fields
    // ------------------------------
private:

    Board board{};
    inline static const char* startingPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // ------------------------------
    // Engine options
    // ------------------------------

    lli _threadCount = 1;
    std::string _debugPath;

    // Options available in engine
    inline static const OptionT<Option::OptionType::spin> Threads{"Threads", _changeThreadCount, 1, 1024, 1};
    inline static const OptionT<Option::OptionType::string> DebugLogFile{"Debug Log File", _changeDebugState, ""};

    inline static const EngineInfo engineInfo = {
        .author = "Jakub Lisowski, Warsaw University of Technology",
        .name = "ChessEngine development version 0.02",
        .options = std::map<std::string, const Option*>({
            std::make_pair("Threads", &Threads),
            std::make_pair("Debug Log File", &DebugLogFile),
        }),
    };
};

#endif //ENGINE_H
