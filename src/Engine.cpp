//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/Engine.h"
#include "../include/MoveGeneration/ChessMechanics.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/OpeningBook/OpeningBook.h"
#include "../include/MoveGeneration/MoveGenerator.h"

void Engine::Initialize()
{
    _board = FenTranslator::GetDefault();
    _startingBoard = _board;
}

void Engine::writeBoard() const { DisplayBoard(_board); }

std::map<std::string, uint64_t> Engine::GetPerft(const int depth)
{
    ChessMechanics game(_board);
    const Board startingBoard = _board;
    std::map<std::string, uint64_t> moveMap{};

    game.IterativeBoardTraversal(
        [&](const Board&bd)
        {
            const auto moveStr = GetLongAlgebraicMoveEncoding(startingBoard, bd);

            uint64_t localSum{};
            game.IterativeBoardTraversal(
                [&]([[maybe_unused]] Board&unused)
                {
                    ++localSum;
                },
                depth - 1
            );

            moveMap[moveStr] = localSum;
        },
        1
    );

    return moveMap;
}

std::map<std::string, uint64_t> Engine::GetMoveBasedPerft(const int depth)
{
    Board startingBoard = _board;
    MoveGenerator game(startingBoard);
    std::map<std::string, uint64_t> moveMap{};

    const auto moves = game.GetMovesFast();

    const auto oldCastling = startingBoard.Castlings;
    const auto oldElPassant = startingBoard.elPassantField;
    for(const auto move : moves)
    {
        Move::MakeMove(move, startingBoard);
        moveMap[move.GetLongAlgebraicNotation()] = game.CountMoves(depth - 1);
        Move::UnmakeMove(move, startingBoard, oldCastling, oldElPassant);
    }

    return moveMap;
}

void Engine::SetFenPosition(const std::string&fenStr)
{
    _board = FenTranslator::Translate(fenStr);
    _startingBoard = _board;
}

const EngineInfo& Engine::GetEngineInfo() { return engineInfo; }

bool Engine::ApplyMoves(const std::vector<std::string>&UCIMoves)
{
    Board workBoard = _startingBoard;

    for (auto&move: UCIMoves)
    {
        const auto [oldPos, newPos] = ExtractPositionsFromEncoding(move);

        if (oldPos == 0 || newPos == 0) return false;
        if (!_applyMove(workBoard, move, oldPos, newPos)) return false;
    }

    _board = workBoard;
    return true;
}

void Engine::RestartEngine()
{
    _board = _startingBoard;
}

Board Engine::GetUnderlyingBoardCopy() const
{
    return _board;
}

std::string Engine::GetFenTranslation() const { return FenTranslator::Translate(_board); }

bool Engine::_applyMove(Board&board, const std::string&move, const uint64_t oldPos, const uint64_t newPos)
{
    const size_t movingColorIndex = board.movColor * Board::BoardsPerCol;
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
    if (const uint64_t promotingMask = board.movColor == WHITE
                                           ? WhitePawnMap::PromotingMask
                                           : BlackPawnMap::PromotingMask;
        movedFigIndex == pawnsIndex && (promotingMask & oldPos) != 0)
    {
        // move does not contain promoted figure encoding
        if (move.size() != 5) return false;

        switch (std::toupper(move[4]))
        {
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
    for (const auto moves = mech.GetPossibleMoveSlow(); const auto&movedBoard: moves)
        // matching board was found
        if ((movedBoard.boards[movingColorIndex + movedFigIndex] & oldPos) == 0
            && (movedBoard.boards[movingColorIndex + destBoardIndex] & newPos) != 0)
        {
            board = movedBoard;
            return true;
        }

    return false;
}

void Engine::_changeDebugState(Engine&eng, std::string&nPath)
{
    GlobalLogger.ChangeLogStream(nPath);
}

// IMPORTANT TODO: what happens when startpos is not basic game?!

void Engine::GoMoveTime(const lli time, const std::vector<std::string>& moves) const
{
    if (const auto& bookMove = _book.GetRandomNextMove(moves); !bookMove.empty())
    {
        GlobalLogger.StartLogging() << std::format("bestmove {}\n", bookMove);
        return;
    }

    BestMoveSearch searchMachine(_board);
    auto bestMove = searchMachine.searchMoveTimeFullBoardEvalUnthreaded<decltype(BoardEvaluator::DefaultFullEvalFunction),
            true>(BoardEvaluator::DefaultFullEvalFunction, time);

    GlobalLogger.StartLogging() << std::format("bestmove {}\n", bestMove);
}

void Engine::GoDepth(const int depth, const std::vector<std::string>& moves) const
{
    if (const auto& bookMove = _book.GetRandomNextMove(moves); !bookMove.empty())
    {
        GlobalLogger.StartLogging() << std::format("bestmove {}\n", bookMove);
        return;
    }

    BestMoveSearch searchMachine(_board);
    auto bestMove = searchMachine.searchMoveDepthFullBoardEvalUnthreaded<decltype(BoardEvaluator::DefaultFullEvalFunction),
        true>(BoardEvaluator::DefaultFullEvalFunction, depth);

    GlobalLogger.StartLogging() << std::format("bestmove {}\n", bestMove);
}
