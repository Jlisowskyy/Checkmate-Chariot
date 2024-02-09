//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/ChessEngine.h"

#include "../include/Interface/UCITranslator.h"
#include "../include/Engine.h"
#include "../include/MoveGeneration/BishopMap.h"
#include "../include/MoveGeneration/KingMap.h"
#include "../include/MoveGeneration/BishopMapGenerator.h"
#include "../include/MoveGeneration/RookMap.h"
#include "../include/MapTypes/ModuloFreeNoOffsetBishopMap.h"
#include "../include/MapTypes/ModuloFreeBishopMap.h"
#include "../include/TestsAndDebugging/MapCorrectnessTest.h"
#include "../include/TestsAndDebugging/MapPerformanceTest.h"


void ChessEngineMainEntry() {
    Engine engine{};
    const UCITranslator translator{engine};

    engine.Initialize();
    translator.BeginCommandTranslation();
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
    ModuloFreeBishopMapNoOffset::ParameterSearch();
}

void RookHashingTest() {
    SimpleRookMap::ParameterSearch();
}

void MapHashTest() {
    MapPerformanceTester::PerformanceTest("/home/Jlisowskyy/Repos/ChessEngine/Tests/perf1", SimpleBishopMap());
}

