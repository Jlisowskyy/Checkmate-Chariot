//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/ChessEngine.h"
#include "../include/UCITranslator.h"
#include "../include/Engine.h"
#include "../include/RookMap.h"

void ChessEngineMainEntry() {
    Engine engine{};
    const UCITranslator translator{engine};

    engine.Initialize();
    translator.BeginCommandTranslation();
}

void ChessHashingTest1() {
    static constexpr const char* rookTest = "R7/7R/8/8/8/3R4/8/8 w - -";
    srand(time(nullptr));

    RookMap rMap{};
    rMap.FindHashParameters();

    // Engine engine{};
    // engine.SetFenPosition(rookTest);
    // engine.writeBoard();

}
