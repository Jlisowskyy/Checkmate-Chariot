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
        if (!_applyMove(workBoard, move))
            return false;

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

bool Engine::_applyMove(Board& board, const std::string& move)
{
    MoveGenerator mech(board, TManager.GetDefaultStack());

    // generating moves
    auto moves = mech.GetMovesFast<false, false>();

    for (size_t i = 0 ; i < moves.size; ++i)
        if (move == moves[i].GetLongAlgebraicNotation())
        {

            Move::MakeMove(moves[i], board);
            TManager.GetDefaultStack().PopAggregate(moves);
            return true;
        }

    TManager.GetDefaultStack().PopAggregate(moves);
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
