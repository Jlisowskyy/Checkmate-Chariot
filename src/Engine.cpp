//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/Engine.h"
#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/MoveGeneration/MoveGenerator.h"
#include "../include/Search/BestMoveSearch.h"
#include "../include/Search/TranspositionTable.h"
#include "../include/Search/ZobristHash.h"
#include "../include/ThreadManagement/GameTimeManager.h"

std::string Engine::_debugEnginePath = Engine::_defaultBookPath;

void Engine::WriteBoard() const { DisplayBoard(_board); }

std::map<std::string, uint64_t> Engine::GetPerft(const int depth)
{
    MoveGenerator mgen{_board, TManager.GetDefaultStack()};
    return mgen.GetCountedMoves(depth);
}

bool Engine::SetFenPosition(const std::string &fenStr)
{
    // Validation whether given fenstr is same as startpos, because of some weird uci implementations not using startpos
    if (fenStr.length() >= _startposPrefix.length() && fenStr.substr(0, _startposPrefix.length()) == _startposPrefix)
    {
        SetStartPos();
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
    _isStartPosPlayed = true;
    _board = _startingBoard = _defaultBoard;
}

const EngineInfo &Engine::GetEngineInfo() { return engineInfo; }

bool Engine::ApplyMoves(const std::vector<std::string> &UCIMoves)
{
    Board workBoard = _startingBoard;
    uint64_t hash   = ZHasher.GenerateHash(workBoard);

    for (auto &move : UCIMoves)
        if (!_applyMove(workBoard, move, hash))
            return false;
    _board = workBoard;

    // applying corresponding age
    _board.Age += static_cast<int>(UCIMoves.size());
    return true;
}

void Engine::RestartEngine()
{
    SetStartPos();

    GameTimeManager::Restart();

    // cleaning tt
    TTable.ClearTable();
}

Board Engine::GetUnderlyingBoardCopy() const { return _board; }

std::string Engine::GetFenTranslation() const { return FenTranslator::Translate(_board); }

bool Engine::_applyMove(Board &board, const std::string &move, uint64_t &hash)
{
    MoveGenerator mech(board, TManager.GetDefaultStack());

    // generating moves
    auto moves = mech.GetMovesFast<false, false>();

    for (size_t i = 0; i < moves.size; ++i)
        if (move == moves[i].GetLongAlgebraicNotation())
        {
            VolatileBoardData data{board};
            hash = ZHasher.UpdateHash(hash, moves[i], data);
            board.Repetitions[hash]++;

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

void Engine::Go(GoInfo &info, const std::vector<std::string> &moves)
{
    if (UseOwnBook && _isStartPosPlayed)
        if (const auto &bookMove = _book.GetRandomNextMove(moves); !bookMove.empty())
        {
            GlobalLogger.LogStream << std::format("bestmove {}\n", bookMove);
            return;
        }

    /*
     * UCI specification says nothing about passing time as 0 value,
     * that is to try rescue ourselves and not violate UCI rules we limit the depth to 3 and sets time to 1
     * */

    if (lli &colTime = _board.MovingColor == WHITE ? info.timeInfo.wTime : info.timeInfo.bTime; colTime == 0)
    {
        colTime    = 1;
        info.depth = std::min(info.depth, 1);

        SearchThreadManager::GoWoutThread(_board, info);
        return;
    }

    TManager.Go(_board, info);
}

void Engine::StopSearch() { TManager.Stop(); }

void Engine::GoInfinite() { TManager.GoInfinite(_board); }

void Engine::_clearHash(Engine &) { TTable.ClearTable(); }

void Engine::_changeDebugEnginePath(Engine &, std::string &path) { _debugEnginePath = path; }
void Engine::_changeBookPath(Engine &engine, std::string &path) { engine._bookPath = path; }

void Engine::PonderHit()
{
    TraceIfFalse(TManager.IsPonderOn(), "Received ponderhit command when no pondering was enabled");

    if (TManager.IsSearchOn() && TManager.IsPonderOn())
    {
        TManager.DisablePonder();
        GameTimeManager::PonderHit(static_cast<Color>(_board.MovingColor), _board);
    }
}

int Engine::GetQuiesceEval()
{
    BestMoveSearch searcher{_board, TManager.GetDefaultStack()};
    return searcher.QuiesceEval() * BoardEvaluator::ScoreGrain;
}

int Engine::GetEvalPrinted()
{
    BoardEvaluatorPrinter::resetEval<EvalMode::PrintMode>();
    BoardEvaluatorPrinter::setBoard<EvalMode::PrintMode>(_board);
    int32_t eval = BoardEvaluator::Evaluation2<EvalMode::PrintMode>(_board);
    BoardEvaluatorPrinter::printAll<EvalMode::PrintMode>();

    return (_board.MovingColor == WHITE ? eval : -eval);
}

Engine::Engine() : _defaultBoard(FenTranslator::GetDefault()) { _board = _startingBoard = _defaultBoard; }
