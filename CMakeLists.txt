################################################################################
#                                CMake init                                    #
################################################################################

cmake_minimum_required(VERSION 3.27)
project(Checkmate-Chariot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# We C++ 20 is required by the main exec

################################################################################
#                             Tunable parameters                               #
################################################################################

# Uncomment to stop enforcing inline on INLINE macro
#add_compile_definitions(PREVENT_FORCE_INLINE=1)

# Uncomment to add gathering TT statistics
add_compile_definitions(TEST_TT=1)

# Uncomment to add displaying debug info about aspiration window
#add_compile_definitions(TEST_ASP_WIN=1)

# Uncomment to allow usage of aspiration windows inside the search
add_compile_definitions(USE_ASP_WIN=1)

# Uncomment to allow tracing every extension applied
#add_compile_definitions(TRACE_EXTENSIONS=1)

################################################################################
#                     Inspecting platform capabilities                         #
################################################################################

cmake_host_system_information(RESULT LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
configure_file(include/cmakeDefines.h.in include/cmakeDefines.h @ONLY)

################################################################################
#                         Fetch GoogleTest from github                         #
################################################################################

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/d83fee138a9ae6cb7c03688a2d08d4043a39815d.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

################################################################################
#                                   Main App                                   #
################################################################################

set(CHECKMATE_CHARIOT_SOURCES
        include/CompilationConstants.h
        include/ChessEngine.h
        src/ChessEngine.cpp
        src/UCITranslator.cpp
        include/Interface/UCIOptions.h
        src/Engine.cpp
        include/Engine.h
        include/EngineUtils.h
        src/FenTranslator.cpp
        include/Interface/FenTranslator.h
        src/Logger.cpp
        include/Interface/Logger.h
        src/EngineUtils.cpp
        include/MoveGeneration/RookMap.h
        include/MoveGeneration/BishopMap.h
        include/MoveGeneration/KnightMap.h
        include/BitOperations.h
        include/MovesHashMap.h
        include/MoveGeneration/KingMap.h
        include/MoveGeneration/MoveGenerationUtils.h
        include/MoveGeneration/QueenMap.h
        include/MoveGeneration/WhitePawnMap.h
        src/ChessMechanics.cpp
        include/MoveGeneration/ChessMechanics.h
        include/Interface/UCITranslator.h
        include/MoveGeneration/RookMapGenerator.h
        include/MoveGeneration/BishopMapGenerator.h
        include/MapTypes/HashFunctions.h
        include/TestsAndDebugging/MapPerformanceTest.h
        include/TestsAndDebugging/MapCorrectnessTest.h
        include/MapTypes/FancyMagicBishopMap.h
        include/MapTypes/FancyMagicRookMap.h
        include/MoveGeneration/SparseRandomGenerator.h
        include/TestsAndDebugging/MoveGenerationTests.h
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
        include/Search/BestMoveSearch.h
        src/BestMoveSearch.cpp
        include/TestsAndDebugging/CsvOperator.h
        include/TestsAndDebugging/SearchPerfTester.h
        src/SearchPerfTester.cpp
        src/CsvOperator.cpp
        include/MoveGeneration/Move.h
        include/Board.h
        src/Board.cpp
        include/MoveGeneration/MoveGenerator.h
        include/ThreadManagement/Stack.h
        include/ThreadManagement/SearchThreadManager.h
        src/SearchThreadManager.cpp
        include/Evaluation/MoveSortEval.h
        include/Search/TranspositionTable.h
        include/Search/ZobristHash.h
        src/ZobristHash.cpp
        src/TranspositionTable.cpp
        include/Evaluation/KillerTable.h
        include/Evaluation/CounterMoveTable.h
        include/Evaluation/HistoricTable.h
        src/MoveGenerator.cpp
        src/FancyMagicRookMap.cpp
        src/FancyMagicBishopMap.cpp
        src/Move.cpp
        src/MapCorrectnessTest.cpp
        src/MapPerformanceTest.cpp
        include/Evaluation/KingSafetyEval.h
        include/MoveGeneration/FileMap.h
        include/Evaluation/StructureEvaluator.h
        src/HistoricTable.cpp
        src/CounterMoveTable.cpp
        include/ThreadManagement/GameTimeManager.h
        src/GameTimeManager.cpp
        include/ThreadManagement/GameTimeManagerUtils.h
        src/GameTimeManagerUtils.cpp
        src/BoardEvaluator.cpp
        include/Evaluation/BoardEvaluatorPrinter.h
        src/BoardEvaluatorPrinter.cpp
        include/TestsAndDebugging/TestSetup.h
        include/TestsAndDebugging/StateReconstructor.h
        include/TestsAndDebugging/DebugTools.h
        src/DebugTools.cpp
)

# Create a library with the core functionality
add_library(Checkmate-Core OBJECT ${CHECKMATE_CHARIOT_SOURCES})

# Create the main executable
add_executable(Checkmate-Chariot main.cpp)
target_link_libraries(Checkmate-Chariot PRIVATE Checkmate-Core)


################################################################################
#                                  Unit Tests                                  #
################################################################################

add_executable(Checkmate-Chariot-Tests
        tests/TestingGoogleTests.cc
        tests/GameTimeManager.cc
        tests/Logger.cc
        tests/UCITranslator.cc
        tests/BoardEvaluator.cc
        tests/ChessMechanics.cc
        tests/SearchTests.cc
)
target_link_libraries(
        Checkmate-Chariot-Tests PRIVATE Checkmate-Core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(Checkmate-Chariot-Tests)

################################################################################
#                                  Main exec                                   #
################################################################################

find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    target_link_libraries(Checkmate-Core PUBLIC OpenMP::OpenMP_CXX)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(Checkmate-Core PUBLIC -g;-fconstexpr-steps=133554432;-fopenmp)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(Checkmate-Core PUBLIC -fconstexpr-steps=133554432;-O3;-march=native;-fopenmp;-funroll-loops;)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(Checkmate-Core PUBLIC -g;-fconstexpr-steps=133554432;-fopenmp;-O2;-march=native;-fopenmp)
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(Checkmate-Core PUBLIC -fconstexpr-ops-limit=133554432;-g;-fopenmp;-O0;)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(Checkmate-Core PUBLIC -O3;-march=native;-fopenmp;-fconstexpr-ops-limit=1335544322 ;-funroll-loops;-fno-peel-loops;-fno-tracer;)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(Checkmate-Core PUBLIC -g;-fconstexpr-ops-limit=133554432;-fopenmp;-O2;-march=native;-fopenmp;)
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    message(FATAL_ERROR "Not suppoerted yet")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(Checkmate-Core PUBLIC /constexpr:steps133554432;/Zi;/openmp)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(Checkmate-Core PUBLIC /O2;/openmp;/constexpr:steps133554432)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(Checkmate-Core PUBLIC /Zi;/constexpr:steps133554432;-fopenmp;/O2;/openmp)
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE")
    endif()

else()
    message(FATAL_ERROR "Unknown compiler")
endif()

################################################################################
#                                  Post build                                  #
################################################################################

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
