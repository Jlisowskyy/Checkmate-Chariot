cmake_minimum_required(VERSION 3.27)
project(Checkmate-Chariot)

set(CMAKE_CXX_STANDARD 20)

add_executable(Checkmate-Chariot main.cpp
        include/compilationConstants.h
        include/ChessEngine.h
        src/ChessEngine.cpp
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
        include/MoveGeneration/WhitePawnMap.h
        src/ChessMechanics.cpp
        include/MoveGeneration/ChessMechanics.h
        include/Interface/UCITranslator.h
        include/MoveGeneration/RookMapGenerator.h
        include/MoveGeneration/BishopMapGenerator.h
        include/MapTypes/SimpleBishopMap.h
        include/MapTypes/HashFunctions.h
        include/MapTypes/SimpleRookMap.h
        include/MapTypes/ModuloFreeBishopMap.h
        include/MapTypes/ModuloFreeRookMap.h
        include/TestsAndDebugging/MapPerformanceTest.h
        include/MapTypes/ModuloFreeNoOffsetRookMap.h
        include/MapTypes/ModuloFreeNoOffsetBishopMap.h
        include/TestsAndDebugging/MapCorrectnessTest.h
        include/MapTypes/FancyMagicBishopMap.h
        include/MapTypes/FancyMagicRookMap.h
        include/MoveGeneration/SparseRandomGenerator.h
        include/TestsAndDebugging/MoveGenerationTests.h
        include/MoveGeneration/PinningMask.h
        include/MoveGeneration/BlackPawnMap.h
        src/MoveGenerationTests.cpp
        include/ParseTools.h
        src/ParseTools.cpp
        include/OpeningBook/OpeningBook.h
        include/OpeningBook/MoveTRIE.h
        src/MoveTRIE.cpp
        src/OpeningBook.cpp
        include/TestsAndDebugging/BookTester.h
        src/BookTester.cpp
        include/Evaluation/BoardEvaluator.h
        src/BoardEvaluator.cpp
        include/Search/BestMoveSearch.h
        src/BestMoveSearch.cpp
        include/TestsAndDebugging/CsvOperator.h
        include/TestsAndDebugging/SearchPerfTester.h
        src/SearchPerfTester.cpp
        src/CsvOperator.cpp
        include/MoveGeneration/Move.h
        include/Board.h
        src/Board.cpp
        src/MoveGenerator.cpp
        include/MoveGeneration/MoveGenerator.h
)

find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    target_link_libraries( Checkmate-Chariot PUBLIC OpenMP::OpenMP_CXX)
endif()

#target_compile_options(Checkmate-Chariot PUBLIC -O3;-march=raptorlake;-fopenmp;-fconstexpr-ops-limit=133554432;-funroll-loops;-fno-peel-loops;-fno-tracer)
target_compile_options(Checkmate-Chariot PUBLIC -fconstexpr-ops-limit=133554432;-g;)


# Define the source file you want to copy
set(SOURCE_FILE "${CMAKE_SOURCE_DIR}/resources/uci_ready_long")

# Define the destination directory (usually where your executable is built)
set(DESTINATION_DIR "${CMAKE_BINARY_DIR}")

# Copy command
add_custom_command(TARGET Checkmate-Chariot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${SOURCE_FILE}"
    "${DESTINATION_DIR}"
    COMMENT "Copying file to build directory"
)
