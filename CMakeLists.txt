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
add_compile_definitions(TEST_ASP_WIN=1)

# Uncomment to allow usage of aspiration windows inside the search
add_compile_definitions(USE_ASP_WIN=1)

# Uncomment to allow tracing every extension applied
#add_compile_definitions(TRACE_EXTENSIONS=1)

# Uncomment to display detailed data collected from search
#add_compile_definitions(COLLECT_SEARCH_DATA=1)

# Uncomment to disable LMR
#add_compile_definitions(DISABLE_LMR=1)

# Uncomment to display statistics about move heuristic eval storing tables
#add_compile_definitions(COLLECT_TABLE_DATA=1)

# Uncomment to treat promos as single move
add_compile_definitions(TREAT_PROMOS_AS_QUEEN_PROMO=1)

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
        include/Search/MoveIterator.h
        src/MoveIterator.cpp
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

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
        target_compile_options(Checkmate-Core PUBLIC -g;-fconstexpr-steps=133554432;-fopenmp;-fsanitize=undefined)
        target_link_options(Checkmate-Chariot PUBLIC -fsanitize=undefined)
    elseif (BUILD_TYPE_UPPER STREQUAL "RELEASE")
        target_compile_options(Checkmate-Core PUBLIC -fconstexpr-steps=133554432;-O3;-march=native;-fopenmp;-funroll-loops;)
    elseif(BUILD_TYPE_UPPER STREQUAL "RELWITHDEBINFO")
        target_compile_options(Checkmate-Core PUBLIC -g;-fconstexpr-steps=133554432;-fopenmp;-O2;-march=native;-fopenmp)
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${BUILD_TYPE_UPPER}")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
        target_compile_options(Checkmate-Core PUBLIC -fconstexpr-ops-limit=133554432;-g;-fopenmp;-O0;)
    elseif (BUILD_TYPE_UPPER STREQUAL "RELEASE")
        target_compile_options(Checkmate-Core PUBLIC -O3;-march=native;-fopenmp;-fconstexpr-ops-limit=1335544322 ;-funroll-loops;-fno-peel-loops;-fno-tracer;)
    elseif(BUILD_TYPE_UPPER STREQUAL "RELWITHDEBINFO")
        target_compile_options(Checkmate-Core PUBLIC -g;-fconstexpr-ops-limit=133554432;-fopenmp;-O2;-march=native;-fopenmp;)
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${BUILD_TYPE_UPPER}")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" OR CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
    if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
        target_compile_options(Checkmate-Core PUBLIC -fconstexpr-ops-limit=133554432;-g;-fopenmp;-O0;)
    elseif (BUILD_TYPE_UPPER STREQUAL "RELEASE")
        target_compile_options(Checkmate-Core PUBLIC -O3;-march=native;-fopenmp ;-funroll-loops;-fno-peel-loops;-fno-tracer;)
    elseif(BUILD_TYPE_UPPER STREQUAL "RELWITHDEBINFO")
        target_compile_options(Checkmate-Core PUBLIC -g;-fconstexpr-ops-limit=133554432;-fopenmp;-O2;-march=native;-fopenmp;)
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${BUILD_TYPE_UPPER}")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
        target_compile_options(Checkmate-Core PUBLIC /constexpr:steps133554432;/Zi;/openmp)
    elseif (BUILD_TYPE_UPPER STREQUAL "RELEASE")
        target_compile_options(Checkmate-Core PUBLIC /O2;/openmp;/constexpr:steps133554432)
    elseif(BUILD_TYPE_UPPER STREQUAL "RELWITHDEBINFO")
        target_compile_options(Checkmate-Core PUBLIC /Zi;/constexpr:steps133554432;-fopenmp;/O2;/openmp)
    else()
        message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${BUILD_TYPE_UPPER}")
    endif()

else()
    message(FATAL_ERROR "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
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
