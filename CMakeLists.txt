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
        include/UCIOptions.h
        src/Engine.cpp
        include/Engine.h
        include/EngineTypeDefs.h
        src/FenTranslator.cpp
        include/FenTranslator.h
        src/Logger.cpp
        include/Logger.h
        src/EngineTypeDefs.cpp
        src/RookMap.cpp
        include/RookMap.h
)
