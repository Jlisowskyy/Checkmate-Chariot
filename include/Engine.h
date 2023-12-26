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
    Engine() {
        engineInfo.author = "Jakub Lisowski, Warsaw University of Technology";
        engineInfo.name = "ChessEngine development version 0.01";
        engineInfo.options = std::map<std::string, Option>({
            std::make_pair("Threads", OptionT<Option::OptionType::spin>("Threads", 1, 1024, 1)),
            std::make_pair("Debug Log File", OptionT<Option::OptionType::string>("Debug Log File", ""))
            });
    }

    void Initialize() {}
    EngineInfo& GetEngineInfo() { return engineInfo; }

    // ------------------------------
    // private fields
    // ------------------------------
private:
    EngineInfo engineInfo{};
};

#endif //ENGINE_H
