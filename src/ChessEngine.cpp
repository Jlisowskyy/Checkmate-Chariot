//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/ChessEngine.h"

#include "../include/BishopMap.h"
#include "../include/UCITranslator.h"
#include "../include/Engine.h"
#include "../include/KingMap.h"
#include "../include/KnightMap.h"
#include "../include/RookMap.h"

void ChessEngineMainEntry() {
    Engine engine{};
    const UCITranslator translator{engine};

    engine.Initialize();
    translator.BeginCommandTranslation();
}

void ChessHashingTest1() {
    static constexpr const char* rookTest = "R7/7R/8/8/8/3R4/8/8 w - -";

    RookMap rMap{};
    rMap.FindHashParameters();

    // Engine engine{};
    // engine.SetFenPosition(rookTest);
    // engine.writeBoard();

}

void ChessHashingTest2() {
    constexpr KingMap map{};

    for (int i = 0; i < 64; ++i) {
        std::cout << "Moves on position: " << ConvertToReversedPos(i) << std::endl;
        DisplayMask(map.GetMoves(i, 0, 0));
    }

}

void ChessHashingTest3() {
    constexpr KnightMap map{};

    for (int i = 0; i < 64; ++i) {
        std::cout << "Moves on position: " << ConvertToReversedPos(i) << std::endl;
        DisplayMask(map.GetMoves(i, 0, 0));
    }

}

void ChessHashingTest4() {
    BishopMap map{};

    map.Tester();
}
