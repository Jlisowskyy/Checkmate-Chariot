//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef ENGINE_H
#define ENGINE_H

#include "UCIOptions.h"

class Engine {
// ------------------------------
    // Class inner types
    // ------------------------------
public:

    struct SearchResult{

    };


    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------
public:
    Engine() {}

    void Initialize() {}

    // ------------------------------
    // Type interaction
    // ------------------------------

    static const EngineInfo& GetEngineInfo() { return engineInfo; }
    void RestartEngine() { std::cout << "ucinewgame result! " << std::endl; }
    void StopSearch() { std::cout << "stop seearch resullt! " << std::endl; }
    void GoPerft() { std::cout << "go perft result! " << std::endl; }
    void GoDepth(lli depth) { std::cout << "go depth resutl: " << depth << std::endl; }
    void GoMovetime(lli time) { std::cout << "go movetime resutl: " << time << std::endl; }
    void GoInfinite() { std::cout << "go infinite result! " << std::endl; }
    void SetFenPosition(const std::string& fenStr) { std::cout << "Received fen pos: " << fenStr << std::endl; }
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
        std::cout << "New debug path: " << nPath << '\n';
    }

    // ------------------------------
    // private fields
    // ------------------------------
private:
    lli _threadCount = 1;
    std::string _debugPath;

    inline static const OptionT<Option::OptionType::spin> Threads{"Threads", _changeThreadCount, 1, 1024, 1};
    inline static const OptionT<Option::OptionType::string> DebugLogFile{"Debug Log File", _changeDebugState, ""};

    inline static const EngineInfo engineInfo = {
        .author = "Jakub Lisowski, Warsaw University of Technology",
        .name = "ChessEngine development version 0.01",
        .options = std::map<std::string, const Option*>({
            std::make_pair("Threads", &Threads),
            std::make_pair("Debug Log File", &DebugLogFile),
        }),
    };
};

#endif //ENGINE_H
