//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef UCITRANSLATOR_H
#define UCITRANSLATOR_H

#include <iostream>

#include "Engine.h"

class UCITranslator {
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------
public:
    UCITranslator(Engine& engine) : engine(engine) {}

    // ------------------------------
    // Clas interaction
    // ------------------------------

    void BeginCommandTranslation() {
        while (true) {
            std::string buffer;
            std::getline(std::cin, buffer);

            if (buffer == "uci") {
                _uciResponse();
            }
            else if (buffer == "isready")
                _isReadyResponse();
        }
    }
    // ------------------------------
    // private methods
    // ------------------------------
private:
    void _uciResponse() const {
        std::cout << "id name " << engine.GetEngineInfo().name << '\n';
        std::cout << "id author " << engine.GetEngineInfo().author << '\n';

        for(const auto& opt: engine.GetEngineInfo().options) {
            std::cout << opt.second;
        }
        std::cout << "uciok" << std::endl;
    }

    void static _isReadyResponse() {
        std::cout << "readyok" << std::endl;
    }

    // ------------------------------
    // private fields
    // ------------------------------

    Engine& engine;
};



#endif //UCITRANSLATOR_H