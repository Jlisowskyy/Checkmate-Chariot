//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/ChessEngine.h"

#include "../include/Interface/UCITranslator.h"
#include "../include/Engine.h"
#include "../include/MoveGeneration/KingMap.h"
#include "../include/MoveGeneration/BishopMapGenerator.h"
#include "../include/MapTypes/ModuloFreeNoOffsetBishopMap.h"
#include "../include/MapTypes/ModuloFreeBishopMap.h"
#include "../include/TestsAndDebugging/MapCorrectnessTest.h"
#include "../include/TestsAndDebugging/MapPerformanceTest.h"
#include "../include/MapTypes/FancyMagicBishopMap.h"
#include "../include/MapTypes/FancyMagicRookMap.h"
#include "../include/MapTypes/ModuloFreeNoOffsetRookMap.h"
#include "../include/MapTypes/ModuloFreeRookMap.h"
#include "../include/MapTypes/SimpleBishopMap.h"
#include "../include/MapTypes/SimpleRookMap.h"

void ChessEngineMainEntry() {
    // Engine engine{};
    // const UCITranslator translator{engine};
    //
    // engine.Initialize();
    // translator.BeginCommandTranslation();

    DisplayMask(WhitePawnMap::GetMoves(ConvertToReversedPos(8), 0));
}

void BishopSimpleMapParamsGen() {
    size_t sizes[64];
    uint64_t primes[64];

    for (size_t i = 0; i < 64; ++i) {
        const int bInd = ConvertToReversedPos(i);
        const int x = bInd % 8;
        const int y = bInd / 8;

        const size_t minimalSize = BishopMapGenerator::PossibleNeighborWoutOverlapCountOnField(x, y);
        const size_t min2Pow = GetRoundedSizePow(minimalSize);
        const size_t mapSize = 1 << min2Pow; // 2^min2Pow
        const uint64_t primeNumber = PrimeNumberMap[min2Pow];

        sizes[i] = mapSize;
        primes[i] = primeNumber;
    }

    for (const auto elem : sizes) std::cout << elem << std::endl;
    for (const auto elem : primes) std::cout << elem << std::endl;
}

void RookSimpleMapParamsGen() {
    size_t sizes[64];
    uint64_t primes[64];

    for (size_t i = 0; i < 64; ++i) {
        const int bInd = ConvertToReversedPos(i);
        const int x = bInd % 8;
        const int y = bInd / 8;

        const size_t minimalSize = RookMapGenerator::PossibleNeighborWoutOverlapCountOnField(x, y);
        const size_t min2Pow = GetRoundedSizePow(minimalSize);
        const size_t mapSize = 1 << min2Pow; // 2^min2Pow
        const uint64_t primeNumber = PrimeNumberMap[min2Pow];

        sizes[i] = mapSize;
        primes[i] = primeNumber;
    }

    for (const auto elem : sizes) std::cout << elem << std::endl;
    for (const auto elem : primes) std::cout << elem << std::endl;
}

void BishopHashingTest() {
    FancyMagicBishopMap::ParameterSearch();
}

void RookHashingTest() {
    // SimpleRookMap::ParameterSearch();
    // ModuloFreeRookMap::ParameterSearch();
    // ModuloFreeNoOffsetRookMap::ParameterSearch();
    FancyMagicRookMap::ParameterSearch();
}

void MapHashTest() {
    std::cout << "________________________________TEST 1______________________________________\n";
    std::cout << "----------------------------SimpleBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", SimpleBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", ModuloFreeBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMapNoOffset----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", ModuloFreeBishopMapNoOffset());
    std::cout << "----------------------------FancyMagicBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", FancyMagicBishopMap());

    std::cout << "\n\n\n________________________________TEST 2______________________________________\n";
    std::cout << "----------------------------SimpleBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf2", SimpleBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf2", ModuloFreeBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMapNoOffset----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf2", ModuloFreeBishopMapNoOffset());
    std::cout << "----------------------------FancyMagicBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf2", FancyMagicBishopMap());

    std::cout << "________________________________TEST 1______________________________________\n";
    std::cout << "----------------------------SimpleBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", SimpleBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", ModuloFreeBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMapNoOffset----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", ModuloFreeBishopMapNoOffset());
    std::cout << "----------------------------FancyMagicBishopMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", FancyMagicBishopMap());

    std::cout << "\n\n\n________________________________TEST 4______________________________________\n";
    // std::cout << "----------------------------SimpleRookMap----------------------------\n";
    // MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf4", SimpleRookMap());
    // std::cout << "----------------------------ModuloFreeRookMap----------------------------\n";
    // MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf4", ModuloFreeRookMap());
    // std::cout << "----------------------------ModuloFreeNoOffsetRookMap----------------------------\n";
    // MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf4", ModuloFreeNoOffsetRookMap());
    std::cout << "----------------------------FancyMagicRookMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf4", FancyMagicRookMap());

    std::cout << "\n\n\n________________________________TEST 5______________________________________\n";
    // std::cout << "----------------------------SimpleRookMap----------------------------\n";
    // MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf5", SimpleRookMap());
    // std::cout << "----------------------------ModuloFreeRookMap----------------------------\n";
    // MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf5", ModuloFreeRookMap());
    // std::cout << "----------------------------ModuloFreeNoOffsetRookMap----------------------------\n";
    // MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf5", ModuloFreeNoOffsetRookMap());
    std::cout << "----------------------------FancyMagicRookMap----------------------------\n";
    MapPerformanceTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf5", FancyMagicRookMap());
}

void MapCorrectnessTest() {
    std::cout << "________________________________TEST 1______________________________________\n";
    std::cout << "----------------------------SimpleBishopMap----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr1", SimpleBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMap----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr1", ModuloFreeBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMapNoOffset----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr1", ModuloFreeBishopMapNoOffset());
    std::cout << "----------------------------FancyMagicBishopMap----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr1", FancyMagicBishopMap());

    std::cout << "\n\n\n________________________________TEST 2______________________________________\n";
    std::cout << "----------------------------SimpleBishopMap----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr2", SimpleBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMap----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr2", ModuloFreeBishopMap());
    std::cout << "----------------------------ModuloFreeBishopMapNoOffset----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr2", ModuloFreeBishopMapNoOffset());
    std::cout << "----------------------------FancyMagicBishopMap----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr2", FancyMagicBishopMap());

    std::cout << "\n\n\n________________________________TEST 3______________________________________\n";
    // std::cout << "----------------------------SimpleRookMap----------------------------\n";
    // MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr3", SimpleRookMap());
    // std::cout << "----------------------------ModuloFreeRookMap----------------------------\n";
    // MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr3", ModuloFreeRookMap());
    // std::cout << "----------------------------ModuloFreeNoOffsetRookMap----------------------------\n";
    // MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr3", ModuloFreeNoOffsetRookMap());
    std::cout << "----------------------------FancyMagicRookMap----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr3", FancyMagicRookMap());

    std::cout << "\n\n\n________________________________TEST 4______________________________________\n";
    // std::cout << "----------------------------SimpleRookMap----------------------------\n";
    // MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr4", SimpleRookMap());
    // std::cout << "----------------------------ModuloFreeRookMap----------------------------\n";
    // MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr4", ModuloFreeRookMap());
    // std::cout << "----------------------------ModuloFreeNoOffsetRookMap----------------------------\n";
    // MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr4", ModuloFreeNoOffsetRookMap());
    std::cout << "----------------------------FancyMagicRookMap----------------------------\n";
    MapCorrectnessTester::PerformTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/corr4", FancyMagicRookMap());


}


