//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/Engine.h"
#include "../include/MoveGeneration/ChessMechanics.h"
#include "../include/Interface/Logger.h"
#include "../include/MoveGeneration/BlackPawnMap.h"
#include "../include/MoveGeneration/WhitePawnMap.h"

void Engine::Initialize() {
    _board = FenTranslator::GetDefault();
    _startingBoard = _board;
}

void Engine::writeBoard() const { std::cout << _board; }

std::map<std::string, uint64_t> Engine::GetPerft(const int depth) {
    ChessMechanics game(_board);
    const Board startingBoard = _board;
    std::map<std::string, uint64_t> moveMap{};

    game.IterativeBoardTraversal(
        [&](const Board& bd)
        {
            auto [oldBoard, newBoard, mType] = FindMove(startingBoard, bd);
            const auto moveStr = GetShortAlgebraicMoveEncoding(_board, oldBoard, newBoard, mType);

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
    _board = FenTranslator::Translate(fenStr);
    _startingBoard = _board;
}

const EngineInfo& Engine::GetEngineInfo() { return engineInfo; }

bool Engine::ApplyMoves(const std::vector<std::string>& UCIMoves) {
    Board workBoard = _startingBoard;

    for (auto& move : UCIMoves) {
        const auto [oldPos, newPos] = ExtractPositionsFromEncoding(move);

        if (oldPos == 0 || newPos == 0) return false;
        if (!_applyMove(workBoard, move, oldPos, newPos)) return false;
    }

    _board = workBoard;
    return true;
}

void Engine::RestartEngine() {
    _board = _startingBoard;
}

Board Engine::GetUnderlyingBoardCopy() const {
    return _board;
}

std::string Engine::GetFenTranslation() const { return FenTranslator::Translate(_board); }

bool Engine::_applyMove(Board& board, const std::string& move, const uint64_t oldPos, const uint64_t newPos) {
    const size_t movingColorIndex = board.movColor*Board::BoardsPerCol;
    size_t movedFigIndex = 6;

    for (size_t i = 0; i < 6; ++i)
        // piece to move found
        if ((board.boards[movingColorIndex + i] & oldPos) != 0)
            movedFigIndex = i;

    // invalid move no figure found
    if (movedFigIndex == 6) return false;

    // fake signal: important property used in further search
    if ((board.boards[movingColorIndex + movedFigIndex] & newPos) != 0) return false;

    // most common situation same board to be placed
    size_t destBoardIndex = movedFigIndex;
    // moving pawn, promotion should be checked additionally
    if (const uint64_t promotingMask = board.movColor == WHITE ? WhitePawnMap::PromotingMask : BlackPawnMap::PromotingMask;
        movedFigIndex == pawnsIndex && (promotingMask & oldPos) != 0)
    {
        // move does not contain promoted figure encoding
        if (move.size() != 5) return false;

        switch (std::toupper(move[4])) {
            case 'R':
                destBoardIndex = rooksIndex;
                break;
            case 'N':
                destBoardIndex = knightsIndex;
                break;
            case 'Q':
                destBoardIndex = queensIndex;
                break;
            case 'B':
                destBoardIndex = bishopsIndex;
                break;
            default:
                return false;
        }
    }

    // Todo: reconsider these two lines below in future:
    Board workBoard = board;
    ChessMechanics mech(workBoard);

    // generating moves
    for (const auto moves = mech.GetPossibleMoveSlow(); const auto& movedBoard : moves)
        // matching board was found
        if ((movedBoard.boards[movingColorIndex + movedFigIndex] & oldPos) == 0
            && (movedBoard.boards[movingColorIndex + destBoardIndex] & newPos) != 0)
        {
            board = movedBoard;
            return true;
        }

    return false;
}

void Engine::_changeDebugState(Engine& eng, std::string& nPath) {
    GlobalLogger.ChangeLogStream(nPath);
}
