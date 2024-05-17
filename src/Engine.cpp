//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/Engine.h"
#include "../include/MoveGeneration/ChessMechanics.h"
#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/Search/TranspositionTable.h"
#include "../include/ThreadManagement/GameTimeManager.h"

std::string Engine::_debugEnginePath = Engine::_defaultBookPath;

void Engine::Initialize()
{
    _board         = FenTranslator::GetDefault();
    _startingBoard = _board;
}

void Engine::WriteBoard() const { DisplayBoard(_board); }

std::map<std::string, uint64_t> Engine::GetPerft(const int depth)
{
    Board startingBoard = _board;
    MoveGenerator game(startingBoard, TManager.GetDefaultStack());
    std::map<std::string, uint64_t> moveMap{};

    const auto moves = game.GetMovesFast();

    const auto oldCastling  = startingBoard.Castlings;
    const auto oldElPassant = startingBoard.ElPassantField;
    for (size_t i = 0; i < moves.size; ++i)
    {
        Move::MakeMove(moves[i], startingBoard);
        moveMap[moves[i].GetLongAlgebraicNotation()] = game.CountMoves(depth - 1);
        Move::UnmakeMove(moves[i], startingBoard, oldCastling, oldElPassant);
    }

    TManager.GetDefaultStack().PopAggregate(moves);
    return moveMap;
}

bool Engine::SetFenPosition(const std::string &fenStr)
{
    // restarting position age
    _age = 1;

    // Validation whether given fenstr is same as startpos, because of some weird uci implementations not using startpos
    if (fenStr.length() >= _startposPrefix.length() && fenStr.substr(0, _startposPrefix.length()) == _startposPrefix)
    {
        _isStartPosPlayed = true;
        _board            = FenTranslator::GetDefault();
        _startingBoard    = _board;
        return true;
    }

    // save previous state
    const bool prevState = _isStartPosPlayed;

    // Trying to parse the fen string
    const bool isParsed = FenTranslator::Translate(fenStr, _board);

    // Change flag state accordingly
    _isStartPosPlayed = !isParsed && prevState;

    // Setup start board
    _startingBoard = _board;

    return isParsed;
}

void Engine::SetStartPos()
{
    // restarting position age
    _age = 1;

    _isStartPosPlayed = true;
    _board            = FenTranslator::GetDefault();
    _startingBoard    = _board;
}

const EngineInfo &Engine::GetEngineInfo() { return engineInfo; }

bool Engine::ApplyMoves(const std::vector<std::string> &UCIMoves)
{
    Board workBoard = _startingBoard;

    for (auto &move : UCIMoves)
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
    _board         = FenTranslator::GetDefault();
    _startingBoard = _board;

    // cleaning tt
    TTable.ClearTable();

    // resetting age
    _age = 1;
}

Board Engine::GetUnderlyingBoardCopy() const { return _board; }

std::string Engine::GetFenTranslation() const { return FenTranslator::Translate(_board); }

bool Engine::_applyMove(Board &board, const std::string &move)
{
    MoveGenerator mech(board, TManager.GetDefaultStack());

    // generating moves
    auto moves = mech.GetMovesFast<false, false>();

    for (size_t i = 0; i < moves.size; ++i)
        if (move == moves[i].GetLongAlgebraicNotation())
        {

            Move::MakeMove(moves[i], board);
            TManager.GetDefaultStack().PopAggregate(moves);
            return true;
        }

    TManager.GetDefaultStack().PopAggregate(moves);
    return false;
}

/// <summary>
/// If a file logger of the engine is not set, creates a new one, otherwise changes the file of the existing one
/// @remark This APPENDS a new file logger to the global logger and monitors it's working file
/// </summary>
void Engine::_changeOrSetLogFile([[maybe_unused]] Engine &eng, std::string &nPath)
{
    if (eng._fileLogger == nullptr)
    {
        eng._fileLogger = std::make_shared<FileLogger>(nPath);
        GlobalLogger.AppendNext(eng._fileLogger);
    }
    else
    {
        eng._fileLogger->ChangeFile(nPath);
    }
}

void Engine::_changeHashSize([[maybe_unused]] Engine &eng, const lli size)
{
    if (TTable.ResizeTable(size) == -1)
        GlobalLogger.LogStream << std::format("[ ERROR ] not able to resize the table with passed size {} MB\n", size);
}

void Engine::_changeBookUsage(Engine &eng, const bool newValue)
{
    if (newValue)
    {
        eng._book.LoadBook(eng._bookPath, OpeningBook::bookFileType::text_uci);
        TraceIfFalse(eng._book.IsLoadedCorrectly(), "Book was not loaded correctly, disabling it");

        if (eng._book.IsLoadedCorrectly())
            eng.UseOwnBook = newValue;
    }
    else
        eng.UseOwnBook = false;
}

void Engine::Go(const GoInfo &info, const std::vector<std::string> &moves)
{
    if (UseOwnBook && _isStartPosPlayed)
        if (const auto &bookMove = _book.GetRandomNextMove(moves); !bookMove.empty())
        {
            GlobalLogger.LogStream << std::format("bestmove {}\n", bookMove);
            return;
        }

    TManager.Go(_board, _age, info);
}

void Engine::StopSearch() { TManager.Stop(); }

void Engine::GoInfinite() { TManager.GoInfinite(_board, _age); }

void Engine::_clearHash(Engine &) { TTable.ClearTable(); }

void Engine::_changeDebugEnginePath(Engine &, std::string &path) { _debugEnginePath = path; }
void Engine::_changeBookPath(Engine &engine, std::string &path) { engine._bookPath = path; }

void Engine::PonderHit()
{
    TraceIfFalse(TManager.IsPonderOn(), "Received ponderhit command when no pondering was enabled");

    if (TManager.IsSearchOn() && TManager.IsPonderOn())
    {
        TManager.DisablePonder();
        GameTimeManager::PonderHit(static_cast<Color>(_board.MovingColor), _board, _age);
    }
}
