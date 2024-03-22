//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/Engine.h"
#include "../include/MoveGeneration/ChessMechanics.h"
#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/OpeningBook/OpeningBook.h"
#include "../include/Search/TranspositionTable.h"

void Engine::Initialize()
{
    _board = FenTranslator::GetDefault();
    _startingBoard = _board;
}

void Engine::writeBoard() const { DisplayBoard(_board); }

std::map<std::string, uint64_t> Engine::GetPerft(const int depth)
{
    Board startingBoard = _board;
    MoveGenerator game(startingBoard, TManager.GetDefaultStack());
    return game.GetCountedMoves(depth);
}

std::map<std::string, uint64_t> Engine::GetMoveBasedPerft(const int depth)
{
    Board startingBoard = _board;
    MoveGenerator game(startingBoard, TManager.GetDefaultStack());
    std::map<std::string, uint64_t> moveMap{};

    const auto moves = game.GetMovesFast();

    const auto oldCastling = startingBoard.Castlings;
    const auto oldElPassant = startingBoard.elPassantField;
    for (size_t i = 0; i < moves.size; ++i)
    {
        Move::MakeMove(moves[i], startingBoard);
        moveMap[moves[i].GetLongAlgebraicNotation()] = game.CountMoves(depth - 1);
        Move::UnmakeMove(moves[i], startingBoard, oldCastling, oldElPassant);
    }

    TManager.GetDefaultStack().PopAggregate(moves);
    return moveMap;
}

void Engine::SetFenPosition(const std::string& fenStr)
{
    // restarting position age
    _age = 1;

    if (fenStr.length() >= _startposPrefix.length() && fenStr.substr(0, _startposPrefix.length()) == _startposPrefix)
    {
        _isStartPosPlayed = true;
        _board = FenTranslator::GetDefault();
        _startingBoard = _board;
        return;
    }

    if (FenTranslator::Translate(fenStr, _board) == false)
        _isStartPosPlayed = true;
    else
        _isStartPosPlayed = false;

    _startingBoard = _board;
}

void Engine::SetStartpos()
{
    // restarting position age
    _age = 1;

    _isStartPosPlayed = true;
    _board = FenTranslator::GetDefault();
    _startingBoard = _board;
}

const EngineInfo& Engine::GetEngineInfo() { return engineInfo; }

bool Engine::ApplyMoves(const std::vector<std::string>& UCIMoves)
{
    Board workBoard = _startingBoard;

    for (auto& move : UCIMoves)
    {
        const auto [oldPos, newPos] = ExtractPositionsFromEncoding(move);

        if (oldPos == 0 || newPos == 0)
            return false;
        if (!_applyMove(workBoard, move, oldPos, newPos))
            return false;
    }

    _board = workBoard;

    // applying corresponding age
    _age = UCIMoves.size() + 1;
    return true;
}

void Engine::RestartEngine()
{
    // enabling book plays
    _isStartPosPlayed = true;

    // loding default board
    _board = FenTranslator::GetDefault();
    _startingBoard = _board;

    // cleaning tt
    TTable.ClearTable();

    // resetting age
    _age = 1;
}

Board Engine::GetUnderlyingBoardCopy() const { return _board; }

std::string Engine::GetFenTranslation() const { return FenTranslator::Translate(_board); }

bool Engine::_applyMove(Board& board, const std::string& move, const uint64_t oldPos, const uint64_t newPos)
{
    const size_t movingColorIndex = board.movColor * Board::BoardsPerCol;
    size_t movedFigIndex = 6;

    for (size_t i = 0; i < 6; ++i)
        // piece to move found
        if ((board.boards[movingColorIndex + i] & oldPos) != 0)
            movedFigIndex = i;

    // invalid move no figure found
    if (movedFigIndex == 6)
        return false;

    // fake signal: important property used in further search
    if ((board.boards[movingColorIndex + movedFigIndex] & newPos) != 0)
        return false;

    // most common situation same board to be placed
    size_t destBoardIndex = movedFigIndex;
    // moving pawn, promotion should be checked additionally
    if (const uint64_t promotingMask =
            board.movColor == WHITE ? WhitePawnMap::PromotingMask : BlackPawnMap::PromotingMask;
        movedFigIndex == pawnsIndex && (promotingMask & oldPos) != 0)
    {
        // move does not contain promoted figure encoding
        if (move.size() != 5)
            return false;

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
    for (const auto moves = mech.GetPossibleMoveSlow(); const auto& movedBoard : moves)
        // matching board was found
        if ((movedBoard.boards[movingColorIndex + movedFigIndex] & oldPos) == 0 &&
            (movedBoard.boards[movingColorIndex + destBoardIndex] & newPos) != 0)
        {
            board = movedBoard;
            return true;
        }

    return false;
}

void Engine::_changeDebugState([[maybe_unused]] Engine& eng, std::string& nPath)
{
    GlobalLogger.ChangeLogStream(nPath);
}

void Engine::_changeHashSize([[maybe_unused]]Engine& eng, const lli size)
{
    if (TTable.ResizeTable(size) == -1)
        GlobalLogger.StartErrLogging()
            << std::format("[ ERROR ] not able to resize the table with passed size {} MB\n", size);
}

void Engine::GoMoveTime(const lli time, const std::vector<std::string>& moves)
{
    if (_book.IsLoadedCorrectly() && _isStartPosPlayed == true)
        if (const auto& bookMove = _book.GetRandomNextMove(moves); !bookMove.empty())
        {
            GlobalLogger.StartLogging() << std::format("bestmove {}\n", bookMove);
            return;
        }

    auto bestMove = TManager.goMoveTime(_board, time, _age);

    GlobalLogger.StartLogging() << std::format("bestmove {}\n", bestMove);
}

void Engine::GoDepth(const int depth, const std::vector<std::string>& moves)
{
    if (_book.IsLoadedCorrectly() && _isStartPosPlayed == true)
        if (const auto& bookMove = _book.GetRandomNextMove(moves); !bookMove.empty())
        {
            GlobalLogger.StartLogging() << std::format("bestmove {}\n", bookMove);
            return;
        }

    auto bestMove = TManager.goDepth(_board, depth, _age);

    GlobalLogger.StartLogging() << std::format("bestmove {}\n", bestMove);
}

void Engine::StopSearch()
{
    if (const auto res = TManager.stop(); !res.empty())
        GlobalLogger.StartLogging() << std::format("bestmove {}\n", res);
}

void Engine::GoInfinite() { TManager.goInfinite(_board, _age); }
