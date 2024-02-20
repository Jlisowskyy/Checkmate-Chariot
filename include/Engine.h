//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>
#include <map>

#include "Interface/UCIOptions.h"
#include "EngineTypeDefs.h"
#include "Interface/FenTranslator.h"
#include "Interface/Logger.h"
#include "MoveGeneration/BlackPawnMap.h"
#include "MoveGeneration/ChessMechanics.h"
#include "MoveGeneration/WhitePawnMap.h"

class Engine {
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------
public:
    Engine() = default;

    void Initialize();

    // ------------------------------
    // Type interaction
    // ------------------------------


    // Implemented function
    void writeBoard() const;
    std::map<std::string, uint64_t> GetPerft(int depth);
    void GoPerft(int depth);
    void SetFenPosition(const std::string& fenStr);
    static const EngineInfo& GetEngineInfo();
    bool ApplyMoves(const std::vector<std::string>& UCIMoves);
    void RestartEngine();

    // TODO: next goals:
    void StopSearch() { std::cout << "stop search resullt! " << std::endl; }
    void GoDepth(lli depth) { std::cout << "go depth resutl: " << depth << std::endl; }
    void GoMovetime(lli time) { std::cout << "go movetime resutl: " << time << std::endl; }
    void GoInfinite() { std::cout << "go infinite result! " << std::endl; }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    static bool _applyMove(Board& board, const std::string& move, uint64_t oldPos, uint64_t newPos);
    static void _changeDebugState(Engine& eng, std::string& nPath);

    static void _changeThreadCount(Engine& eng, const lli tCount) {
        std::cout << "New thread count: " << tCount << '\n';
    }

    // ------------------------------
    // private fields
    // ------------------------------

    Board _board{};
    Board _startingBoard{};


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
