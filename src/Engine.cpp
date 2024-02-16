//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/Engine.h"
#include "../include/MoveGeneration/ChessMechanics.h"

void Engine::Initialize() {
    board = FenTranslator::Translate(startingPosition);
}

void Engine::writeBoard() const { std::cout << board; }

std::map<std::string, uint64_t> Engine::GetPerft(const int depth) {
    ChessMechanics game(board);
    const Board startingBoard = board;
    std::map<std::string, uint64_t> moveMap{};

    game.IterativeBoardTraversal(
        [&](const Board& bd)
        {
            auto [oldBoard, newBoard, mType] = FindMove(startingBoard, bd);
            const auto moveStr = GetShortAlgebraicMoveEncoding(board, oldBoard, newBoard, mType);

            uint64_t localSum{};
            game.IterativeBoardTraversal(
                [&]([[maybe_unused]] Board& unused) {
                    ++localSum;
                },
                depth-1
            );

            moveMap[moveStr] = localSum;
        },
        1
    );

    return moveMap;
}

void Engine::GoPerft(const int depth) {
    const auto t1 = std::chrono::steady_clock::now();
    auto moves = GetPerft(depth);
    const auto t2 = std::chrono::steady_clock::now();

    uint64_t totalSum{};
    for (const auto& [moveStr, moveCount] : moves) {
        GlobalLogger.StartLogging() << std::format("{}: {}\n", moveStr, moveCount);
        totalSum += moveCount;
    }

    GlobalLogger.StartLogging() << std::format("Calculated moves: {} in time: {}ms\n", totalSum, (t2-t1).count()*1e-6);
}

void Engine::SetFenPosition(const std::string& fenStr) {
    board = FenTranslator::Translate(fenStr);
}

const EngineInfo& Engine::GetEngineInfo() { return engineInfo; }
