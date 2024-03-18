//
// Created by Jlisowskyy on 12/26/23.
//

#include <cstdlib>

#include "../include/Interface/Logger.h"
#include "../include/Interface/UCITranslator.h"
#include "../include/ParseTools.h"
#include "../include/TestsAndDebugging/MoveGenerationTests.h"
#include "../include/TestsAndDebugging/SearchPerfTester.h"

UCITranslator::UCICommand UCITranslator::BeginCommandTranslation(std::istream& input)
{
    auto lastCommand = UCICommand::InvalidCommand;
    std::string recordBuffer;

    while (lastCommand != UCICommand::quitCommand && std::getline(input, recordBuffer))
    {
        lastCommand = _cleanMessage(recordBuffer);

        if (lastCommand == UCICommand::InvalidCommand)
            GlobalLogger.StartErrLogging() << "[ ERROR ] Error uccured during translation or execution.\n Refer to UCI "
                                              "protocl manual to get more detailed information.\n";
    }

    return lastCommand;
}

UCITranslator::UCICommand UCITranslator::_cleanMessage(const std::string& buffer)
{
    using funcT = UCICommand (UCITranslator::*)(const std::string&);
    static std::unordered_map<std::string, funcT> CommandBuff{
        {std::string("uci"), &UCITranslator::_uciResponse},
        {std::string("isready"), &UCITranslator::_isReadyResponse},
        {std::string("setoption"), &UCITranslator::_setoptionResponse},
        {std::string("ucinewgame"), &UCITranslator::_ucinewgameResponse},
        {std::string("position"), &UCITranslator::_positionResponse},
        {std::string("go"), &UCITranslator::_goResponse},
        {std::string("stop"), &UCITranslator::_stopResponse},
        {std::string("quit"), &UCITranslator::_quitResponse},
        {std::string("exit"), &UCITranslator::_quitResponse},
        {std::string("d"), &UCITranslator::_displayResponse},
        {std::string("display"), &UCITranslator::_displayResponse},
        {std::string("disp"), &UCITranslator::_displayResponse},
        {std::string("fen"), &UCITranslator::_displayFenResponse},
        {std::string("help"), &UCITranslator::_displayHelpResponse},
        {std::string("clear"), &UCITranslator::_clearConsole},
        {std::string("clean"), &UCITranslator::_clearConsole},
        {std::string("cls"), &UCITranslator::_clearConsole},
    };

    std::string workStr;
    size_t pos = 0;

    while ((pos = ParseTools::ExtractNextWord(buffer, workStr, pos)) != 0)
        if (auto iter = CommandBuff.find(workStr); iter != CommandBuff.end())
            return (this->*(iter->second))(buffer.substr(pos));

    return UCICommand::InvalidCommand;
}

UCITranslator::UCICommand UCITranslator::_stopResponse([[maybe_unused]] const std::string&)
{
    _engine.StopSearch();
    return UCICommand::stopCommand;
}

UCITranslator::UCICommand UCITranslator::_goResponse(const std::string& str)
{
    std::string workStr;
    size_t pos = ParseTools::ExtractNextWord(str, workStr, 0);
    if (pos == 0)
        return UCICommand::InvalidCommand;

    if (workStr == "perft")
    {
        std::string depthStr{};
        pos = ParseTools::ExtractNextWord(str, depthStr, pos);
        if (pos == 0)
            return UCICommand::InvalidCommand;

        int depth;
        try
        {
            depth = std::stoi(depthStr);
        }
        catch (const std::exception& exc)
        {
            return UCICommand::InvalidCommand;
        }

        _engine.GoPerft(depth);
    }
    else if (workStr == "debug")
    {
        std::string depthStr{};
        pos = ParseTools::ExtractNextWord(str, depthStr, pos);
        if (pos == 0)
            return UCICommand::InvalidCommand;

        int depth;
        try
        {
            depth = std::stoi(depthStr);
        }
        catch (const std::exception& exc)
        {
            return UCICommand::InvalidCommand;
        }

        const MoveGenerationTester tester;
        [[maybe_unused]] auto unused = tester.PerformSingleShallowTest(_fenPosition, depth, _appliedMoves, true);
    }
    else if (workStr == "deepDebug")
    {
        std::string depthStr{};
        pos = ParseTools::ExtractNextWord(str, depthStr, pos);
        if (pos == 0)
            return UCICommand::InvalidCommand;

        int depth;
        try
        {
            depth = std::stoi(depthStr);
        }
        catch (const std::exception& exc)
        {
            return UCICommand::InvalidCommand;
        }

        const MoveGenerationTester tester;
        tester.PerformDeepTest(_fenPosition, depth, _appliedMoves);
    }
    else if (workStr == "fullDebug")
    {
        std::string depthStr{};
        pos = ParseTools::ExtractNextWord(str, depthStr, pos);
        if (pos == 0)
            return UCICommand::InvalidCommand;

        int depth;
        try
        {
            depth = std::stoi(depthStr);
        }
        catch (const std::exception& exc)
        {
            return UCICommand::InvalidCommand;
        }

        const MoveGenerationTester tester;
        tester.PerformFullTest(_fenPosition, depth, _appliedMoves);
    }
    else if (workStr == "file")
    {
        std::string path{};
        ParseTools::ExtractNextWord(str, path, pos);
        const MoveGenerationTester tester;
        const bool result = tester.PerformSeriesOfDeepTestFromFile(path);

        if (result == false)
            return UCICommand::InvalidCommand;
    }
    else if (workStr == "infinite")
        _engine.GoInfinite();
    else if (workStr == "depth")
    {
        pos = ParseTools::ExtractNextWord(str, workStr, pos);
        if (pos == 0)
            return UCICommand::InvalidCommand;
        const lli arg = ParseTools::ParseTolli(workStr);
        if (arg <= 0)
            return UCICommand::InvalidCommand;

        _engine.GoDepth(arg, _appliedMoves);
    }
    else if (workStr == "movetime")
    {
        pos = ParseTools::ExtractNextWord(str, workStr, pos);
        if (pos == 0)
            return UCICommand::InvalidCommand;
        const lli arg = ParseTools::ParseTolli(workStr);
        if (arg <= 0)
            return UCICommand::InvalidCommand;

        _engine.GoMoveTime(arg, _appliedMoves);
    }
    else if (workStr == "perfComp")
    {
        std::string file1Str{};
        std::string file2Str{};
        pos = ParseTools::ExtractNextWord(str, file1Str, pos);
        if (pos != 0)
            ParseTools::ExtractNextWord(str, file2Str, pos);

        const MoveGenerationTester tester;
        bool result = tester.PerformPerformanceTest(file1Str, file2Str);
        if (result == false)
            return UCICommand::InvalidCommand;
    }
    else if (workStr == "searchPerf")
    {
        std::string file1Str{};
        std::string file2Str{};
        pos = ParseTools::ExtractNextWord(str, file1Str, pos);
        if (pos != 0)
            ParseTools::ExtractNextWord(str, file2Str, pos);

        bool result = SearchPerfTester::PerformSearchPerfTest(file1Str, file2Str, _engine.TManager.GetDefaultStack());
        if (result == false)
            return UCICommand::InvalidCommand;
    }

    return UCICommand::goCommand;
}

UCITranslator::UCICommand UCITranslator::_positionResponse(const std::string& str)
{
    std::string workStr;
    size_t pos = ParseTools::ExtractNextWord(str, workStr, 0);
    if (pos == 0)
        return UCICommand::InvalidCommand;

    const size_t movesCord = str.find("moves", pos);

    if (workStr == "fen")
    {
        _fenPosition = movesCord == std::string::npos ? ParseTools::GetTrimmed(str.substr(pos))
                                                      : ParseTools::GetTrimmed(str.substr(pos, movesCord - pos));

        _engine.SetFenPosition(_fenPosition);
    }
    else if (workStr == "startpos")
        _engine.SetStartpos();
    else
        return UCICommand::InvalidCommand;

    if (movesCord != std::string::npos)
    {
        pos = movesCord + 5;

        const std::vector<std::string> movesVect = ParseTools::Split(str, pos);

        if (!_engine.ApplyMoves(movesVect))
            return UCICommand::InvalidCommand;

        _appliedMoves = movesVect;
    }
    else
        _appliedMoves.clear();

    return UCICommand::positionCommand;
}

UCITranslator::UCICommand UCITranslator::_ucinewgameResponse([[maybe_unused]] const std::string& unused)
{
    _engine.RestartEngine();
    _appliedMoves.clear();
    TTable.ClearTable();
    return UCICommand::ucinewgameCommand;
}

UCITranslator::UCICommand UCITranslator::_setoptionResponse(const std::string& str)
{
    std::string workStr;
    size_t pos = ParseTools::ExtractNextWord(str, workStr, 0);
    if (pos == 0 || workStr != "name")
        return UCICommand::InvalidCommand;

    std::string optionName{};
    while ((pos = ParseTools::ExtractNextWord(str, workStr, pos)) != 0 && workStr != "value")
    {
        optionName += workStr + ' ';
    }

    // space cleaning
    if (!optionName.empty())
        optionName.pop_back();

    // TODO: Consider error mesage here - unrecognized option
    if (!Engine::GetEngineInfo().options.contains(optionName))
        return UCICommand::InvalidCommand;

    // argument option
    std::string arg = workStr != "value" ? std::string() : ParseTools::GetTrimmed(str.substr(pos));
    if (Engine::GetEngineInfo().options.at(optionName)->TryChangeValue(arg, _engine))
        return UCICommand::setoptionCommand;
    return UCICommand::InvalidCommand;
}

UCITranslator::UCICommand UCITranslator::_uciResponse([[maybe_unused]] const std::string& unused)
{
    GlobalLogger.StartLogging() << "id name " << Engine::GetEngineInfo().name << '\n';
    GlobalLogger.StartLogging() << "id author " << Engine::GetEngineInfo().author << '\n';

    for (const auto& opt : Engine::GetEngineInfo().options)
    {
        GlobalLogger.StartLogging() << *opt.second;
    }
    GlobalLogger.StartLogging() << "uciok" << std::endl;
    return UCICommand::uciCommand;
}

UCITranslator::UCICommand UCITranslator::_isReadyResponse([[maybe_unused]] const std::string& unused)
{
    GlobalLogger.StartLogging() << "readyok" << std::endl;
    return UCICommand::isreadyCommand;
}

UCITranslator::UCICommand UCITranslator::_displayResponse([[maybe_unused]] const std::string& unused)
{
    _engine.writeBoard();
    return UCICommand::displayCommand;
}

UCITranslator::UCICommand UCITranslator::_displayHelpResponse([[maybe_unused]] const std::string& unused)
{
    static auto CustomCommands =
        "In addition to standard UCI commands, these are implemented:\n"
        "- go perft \"depth\" - Simple PERFT test.\n"
        "- go debug \"depth\" - debugging tool reporting first occured error in comparison to any engine\n"
        "                which implements \"go perft command\" - default target engine is stockfish\n"
        "- go deepDebug \"depth\" - debugging tool, which is used to possibly identify invalid move chains which "
        "produces\n"
        "                 buggy result.\n"
        "- go fullDebug \"depth\" - traverses whole tree and invokes simple debug test on each leaf parent to check\n"
        "                 move correctnes on lowest level possible. Insanly slow - use only for lower search. Could be "
        "optimised.\n"
        "- fen - simply displays fen encoding of current map\n"
        "- go perfComp \"input file\" \"output file\" - generates csv file to \"output file\" which contains "
        "information\n"
        "                 about results of simple comparison tests, which uses external engine times to get results\n"
        "- go file \"input file\" - performs series of deepDebug on each positions saved inside input file. For "
        "simplicity\n"
        "                \"input file\" must be containg csv records in given manner: \"fen position\", \"depth\"\n"
        "- go searchPerf \"input file \" \"output file \" - runs straight alpha beta prunning performance tests "
        "               on the framework.\n"
        "Where \"depth\" is integer value indicating layers of traversed move tree.\n\n\n"
        "Additional notes:\n"
        "   - \"go file / \" - will run tests on singlePos.csv\n"
        "   - \"go file\" - will run tests on positionTests.csv\n"
        "   - \"go perfComp /\" - will run tests on perfTest1.csv\n"
        "   - \"go searchPerf\" - will run tests on searchTests.csv";

    GlobalLogger.StartLogging() << "Help content:\n\n"
                                << "TODO MAIN HELP\n\n"
                                << CustomCommands;

    return UCICommand::helpCommand;
}

UCITranslator::UCICommand UCITranslator::_quitResponse([[maybe_unused]] const std::string& unused)
{
    return UCICommand::quitCommand;
}

UCITranslator::UCICommand UCITranslator::_clearConsole([[maybe_unused]] const std::string& unused)
{
#ifdef __unix__
    system("clear");
#else
    system("cls");
#endif

    return UCICommand::displayCommand;
}

UCITranslator::UCICommand UCITranslator::_displayFenResponse([[maybe_unused]] const std::string& unused)
{
    GlobalLogger.StartLogging() << "Acquired fen translation:\n" << _engine.GetFenTranslation() << '\n';
    return UCICommand::displayCommand;
}
