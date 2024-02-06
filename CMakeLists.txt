cmake_minimum_required(VERSION 3.27)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(ChessEngine main.cpp
        include/compilationConstants.h
        include/ChessEngine.h
        src/ChessEngine.cpp
        src/SearchThreadCordination.cpp
        include/SearchThreadCordination.h
        src/UCITranslator.cpp
        include/Interface/UCIOptions.h
        src/Engine.cpp
        include/Engine.h
        include/EngineTypeDefs.h
        src/FenTranslator.cpp
        include/Interface/FenTranslator.h
        src/Logger.cpp
        include/Interface/Logger.h
        src/EngineTypeDefs.cpp
        include/MoveGeneration/RookMap.h
        include/MoveGeneration/BishopMap.h
        include/MoveGeneration/KnightMap.h
        include/BitOperations.h
        include/movesHashMap.h
        include/MoveGeneration/KingMap.h
        include/MoveGeneration/MoveGeneration.h
        include/MoveGeneration/QueenMap.h
        include/MoveGeneration/PawnMap.h
        src/Evaluator.cpp
        include/Evaluator.h
        src/SearchingMachine.cpp
        include/SearchingMachine.h
        src/ChessMechanics.cpp
        include/ChessMechanics.h
        src/EngineTests.cpp
        include/EngineTests.h
        include/Interface/UCITranslator.h
        include/MoveGeneration/RookMapGenerator.h
        include/MoveGeneration/BishopMapGenerator.h
        include/MapTypes/SimpleBishopMap.h
        include/MapTypes/MapScheme.h
        include/MapTypes/HashFunctions.h
)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ChessEngine PUBLIC OpenMP::OpenMP_CXX)
endif()

target_compile_options(ChessEngine  PUBLIC -O3;-march=native;-fopenmp)
#target_compile_options(ChessEngine PUBLIC -fopenmp;-g)