//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef ENGINE_H
#define ENGINE_H

#include "UCIOptions.h"

class Engine {
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

    inline static const EngineInfo engineInfo = {
        .author = "Jakub Lisowski, Warsaw University of Technology",
        .name = "ChessEngine development version 0.01",
        .options = std::map<std::string, Option>({
            std::make_pair("Threads", OptionT<Option::OptionType::spin>("Threads", _changeThreadCount, 1, 1024, 1)),
            std::make_pair("Debug Log File", OptionT<Option::OptionType::string>("Debug Log File", _changeDebugState, ""))
            })
    };
};

#endif //ENGINE_H
