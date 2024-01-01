//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/ChessEngine.h"

#include "../include/MoveGeneration/BishopMap.h"
#include "../include/Interface/UCITranslator.h"
#include "../include/Engine.h"
#include "../include/MoveGeneration/KingMap.h"
#include "../include/MoveGeneration/KnightMap.h"
#include "../include/MoveGeneration/PawnMap.h"
#include "../include/MoveGeneration/RookMap.h"

void ChessEngineMainEntry() {
    Engine engine{};
    const UCITranslator translator{engine};

    engine.Initialize();
    translator.BeginCommandTranslation();
}

void ChessHashingTest1() {
    static constexpr const char* rookTest = "R7/7R/8/8/8/3R4/8/8 w - -";

    RookMapGenerator::FindHashParameters();

    // Engine engine{};
    // engine.SetFenPosition(rookTest);
    // engine.writeBoard();

}

void ChessHashingTest2() {
    for (int i = 0; i < 64; ++i) {
        std::cout << "Moves on position: " << ConvertToReversedPos(i) << std::endl;
        DisplayMask(KingMap::GetMoves(i));
    }

}

void ChessHashingTest3() {
    for (int i = 0; i < 64; ++i) {
        std::cout << "Moves on position: " << ConvertToReversedPos(i) << std::endl;
        DisplayMask(KnightMap::GetMoves(i));
    }
}

void ChessHashingTest4() {
    BishopMapGenerator::FindHashParameters();
}

void ChessHashingTest5() {

}
