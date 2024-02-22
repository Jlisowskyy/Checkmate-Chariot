//
// Created by Jlisowskyy on 12/26/23.
//

#include "../include/Interface/UCITranslator.h"
#include "../include/ParseTools.h"
#include "../include/TestsAndDebugging/MoveGenerationTests.h"
#include "../include/Interface/Logger.h"

UCITranslator::UCICommand UCITranslator::BeginCommandTranslation(std::istream&input)
{
    auto lastCommand = UCICommand::InvalidCommand;
    std::string recordBuffer;

    while (lastCommand != UCICommand::quitCommand && std::getline(input, recordBuffer))
    {
        lastCommand = _cleanMessage(recordBuffer);

        if (lastCommand == UCICommand::InvalidCommand)
            GlobalLogger.StartErrLogging() <<
                    "[ ERROR ] Error uccured during translation or execution.\n Refer to UCI protocl manual to get more detailed information.\n";
    }

    return lastCommand;
}

UCITranslator::UCICommand UCITranslator::_cleanMessage(const std::string&buffer)
{
    std::string workStr;
    size_t pos = 0;

    while ((pos = ParseTools::ExtractNextWord(buffer, workStr, pos)) != 0)
    {
        if (workStr == "uci")
            return _uciResponse();
        if (workStr == "isready")
            return _isReadyResponse();
        if (workStr == "setoption")
            return _setoptionResponse(buffer.substr(pos));
        if (workStr == "ucinewgame")
            return _ucinewgameResponse();
        if (workStr == "position")
            return _positionResponse(buffer.substr(pos));
        if (workStr == "go")
            return _goResponse(buffer.substr(pos));
        if (workStr == "stop")
            return _stopResponse();
        if (workStr == "quit" || workStr == "exit")
            return UCICommand::quitCommand;
        if (workStr == "display" || workStr == "d")
            return _displayResponse();
        if (workStr == "fen")
            return _displayFen();
    }

    return UCICommand::InvalidCommand;
}

UCITranslator::UCICommand UCITranslator::_stopResponse() const
{
    _engine.StopSearch();
    return UCICommand::stopCommand;
}

UCITranslator::UCICommand UCITranslator::_goResponse(const std::string&str) const
{
    std::string workStr;
    size_t pos = ParseTools::ExtractNextWord(str, workStr, 0);
    if (pos == 0) return UCICommand::InvalidCommand;

    if (workStr == "perft")
    {
        std::string depthStr{};
        pos = ParseTools::ExtractNextWord(str, depthStr, pos);
        if (pos == 0) return UCICommand::InvalidCommand;

        int depth;
        try { depth = std::stoi(depthStr); }
        catch (const std::exception&exc) { return UCICommand::InvalidCommand; }

        _engine.GoPerft(depth);
    }
    else if (workStr == "debug")
    {
        std::string depthStr{};
        pos = ParseTools::ExtractNextWord(str, depthStr, pos);
        if (pos == 0) return UCICommand::InvalidCommand;

        int depth;
        try { depth = std::stoi(depthStr); }
        catch (const std::exception&exc) { return UCICommand::InvalidCommand; }

        const MoveGenerationTester tester;
        [[maybe_unused]] tester.PerformSingleShallowTest(_fenPosition, depth, _appliedMoves, true);
    }
    else if (workStr == "deepDebug")
    {
        std::string depthStr{};
        pos = ParseTools::ExtractNextWord(str, depthStr, pos);
        if (pos == 0) return UCICommand::InvalidCommand;

        int depth;
        try { depth = std::stoi(depthStr); }
        catch (const std::exception&exc) { return UCICommand::InvalidCommand; }

        const MoveGenerationTester tester;
        tester.PerformDeepTest(_fenPosition, depth, _appliedMoves);
    }
    else if (workStr == "fullDebug")
    {
        std::string depthStr{};
        pos = ParseTools::ExtractNextWord(str, depthStr, pos);
        if (pos == 0) return UCICommand::InvalidCommand;

        int depth;
        try { depth = std::stoi(depthStr); }
        catch (const std::exception&exc) { return UCICommand::InvalidCommand; }

        const MoveGenerationTester tester;
        tester.PerformFullTest(_fenPosition, depth, _appliedMoves);
    }
    else if (workStr == "file")
    {
        std::string path{};
        ParseTools::ExtractNextWord(str, path, pos);
        const MoveGenerationTester tester;
        const bool result = tester.PerformSeriesOfDeepTestFromFile(path);

        if (result == false) return UCICommand::InvalidCommand;
    }
    else if (workStr == "infinite")
        _engine.GoInfinite();
    else if (workStr == "depth")
    {
        pos = ParseTools::ExtractNextWord(str, workStr, pos);
        if (pos == 0) return UCICommand::InvalidCommand;
        const lli arg = ParseTools::ParseTolli(workStr);
        if (arg <= 0) return UCICommand::InvalidCommand;

        _engine.GoMovetime(arg);
    }
    else if (workStr == "movetime")
    {
        pos = ParseTools::ExtractNextWord(str, workStr, pos);
        if (pos == 0) return UCICommand::InvalidCommand;
        const lli arg = ParseTools::ParseTolli(workStr);
        if (arg <= 0) return UCICommand::InvalidCommand;

        _engine.GoMovetime(arg);
    }
    else if (workStr == "perfComp")
    {
        std::string file1Str{};
        std::string file2Str{};
        pos = ParseTools::ExtractNextWord(str, file1Str, pos);
        if (pos != 0) ParseTools::ExtractNextWord(str, file2Str, pos);

        const MoveGenerationTester tester;
        bool result = tester.PerformPerformanceTest(file1Str, file2Str);
        if (result == false)
            return UCICommand::InvalidCommand;
    }

    return UCICommand::goCommand;
}

UCITranslator::UCICommand UCITranslator::_positionResponse(const std::string&str)
{
    std::string workStr;
    size_t pos = ParseTools::ExtractNextWord(str, workStr, 0);
    if (pos == 0) return UCICommand::InvalidCommand;

    const size_t movesCord = str.find("moves", pos);

    if (workStr == "fen")
    {
        _fenPosition = movesCord == std::string::npos
                           ? ParseTools::GetTrimmed(str.substr(pos))
                           : ParseTools::GetTrimmed(str.substr(pos, movesCord - pos));

        _engine.SetFenPosition(_fenPosition);
    }
    else if (workStr != "startpos")
        return UCICommand::InvalidCommand;

    if (movesCord != std::string::npos)
    {
        pos = movesCord + 5;

        std::vector<std::string> movesVect{};
        while ((pos = ParseTools::ExtractNextWord(str, workStr, pos)) != 0)
        {
            movesVect.push_back(workStr);
        }

        if (!_engine.ApplyMoves(movesVect))
            return UCICommand::InvalidCommand;

        _appliedMoves = movesVect;
    }

    return UCICommand::positionCommand;
}

UCITranslator::UCICommand UCITranslator::_ucinewgameResponse()
{
    _engine.RestartEngine();
    _appliedMoves.clear();
    return UCICommand::ucinewgameCommand;
}

UCITranslator::UCICommand UCITranslator::_setoptionResponse(const std::string&str) const
{
    std::string workStr;
    size_t pos = ParseTools::ExtractNextWord(str, workStr, 0);
    if (pos == 0 || workStr != "name") return UCICommand::InvalidCommand;

    std::string optionName{};
    while ((pos = ParseTools::ExtractNextWord(str, workStr, pos)) != 0 && workStr != "value")
    {
        optionName += workStr + ' ';
    }

    // space cleaning
    if (!optionName.empty()) optionName.pop_back();

    // TODO: Consider error mesage here - unrecognized option
    if (!Engine::GetEngineInfo().options.contains(optionName))
        return UCICommand::InvalidCommand;

    // argument option
    std::string arg = workStr != "value" ? std::string() : ParseTools::GetTrimmed(str.substr(pos));
    if (Engine::GetEngineInfo().options.at(optionName)->TryChangeValue(arg, _engine))
        return UCICommand::setoptionCommand;
    return UCICommand::InvalidCommand;
}

UCITranslator::UCICommand UCITranslator::_uciResponse()
{
    GlobalLogger.StartLogging() << "id name " << Engine::GetEngineInfo().name << '\n';
    GlobalLogger.StartLogging() << "id author " << Engine::GetEngineInfo().author << '\n';

    for (const auto&opt: Engine::GetEngineInfo().options)
    {
        GlobalLogger.StartLogging() << *opt.second;
    }
    GlobalLogger.StartLogging() << "uciok" << std::endl;
    return UCICommand::uciCommand;
}

UCITranslator::UCICommand UCITranslator::_isReadyResponse()
{
    GlobalLogger.StartLogging() << "readyok" << std::endl;
    return UCICommand::isreadyCommand;
}

UCITranslator::UCICommand UCITranslator::_displayResponse() const
{
    _engine.writeBoard();
    return UCICommand::displayCommand;
}

UCITranslator::UCICommand UCITranslator::_displayFen() const
{
    GlobalLogger.StartLogging() << "Acquired fen translation:\n" << _engine.GetFenTranslation() << '\n';
    return UCICommand::displayCommand;
}
