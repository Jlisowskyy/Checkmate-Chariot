//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/ChessEngine.h"
#include "../include/UCITranslator.h"
#include "../include/Engine.h"

void ChessEngineMainEntry() {
    Engine engine{};
    UCITranslator translator{engine};

    engine.Initialize();
    translator.BeginCommandTranslation();
}
