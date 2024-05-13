//
// Created by Jlisowskyy on 12/26/23.
//

#include <cstdlib>
#include <format>

#include "../include/Interface/Logger.h"
#include "../include/Interface/UCITranslator.h"
#include "../include/ParseTools.h"
#include "../include/TestsAndDebugging/MoveGenerationTests.h"
#include "../include/TestsAndDebugging/SearchPerfTester.h"
#include "../include/ThreadManagement/GameTimeManagerUtils.h"

UCITranslator::UCICommand UCITranslator::BeginCommandTranslation(std::istream &input)
{
    auto lastCommand = UCICommand::InvalidCommand;
    std::string recordBuffer;

    while (lastCommand != UCICommand::quitCommand && std::getline(input, recordBuffer))
    {
        lastCommand = _dispatchCommands(recordBuffer);

        if (lastCommand == UCICommand::InvalidCommand)
            GlobalLogger.LogStream << "[ ERROR ] Error uccured during translation or execution.\n Refer to UCI "
                                      "protocl manual to get more detailed information.\n";
    }

    WrapTraceMsgInfo("UCI command translation finished.");

    return lastCommand;
}

UCITranslator::UCICommand UCITranslator::_dispatchCommands(const std::string &buffer)
{
    using funcT = UCICommand (UCITranslator::*)(const std::string &);
    static std::unordered_map<std::string, funcT> CommandBuff{
        {       "uci",         &UCITranslator::_uciResponse},
        {   "isready",     &UCITranslator::_isReadyResponse},
        { "setoption",   &UCITranslator::_setoptionResponse},
        {"ucinewgame",  &UCITranslator::_ucinewgameResponse},
        {  "position",    &UCITranslator::_positionResponse},
        {        "go",          &UCITranslator::_goResponse},
        {      "stop",        &UCITranslator::_stopResponse},
        {      "quit",        &UCITranslator::_quitResponse},
        {      "exit",        &UCITranslator::_quitResponse},
        {         "d",     &UCITranslator::_displayResponse},
        {   "display",     &UCITranslator::_displayResponse},
        {      "disp",     &UCITranslator::_displayResponse},
        {       "fen",  &UCITranslator::_displayFenResponse},
        {      "help", &UCITranslator::_displayHelpResponse},
        {     "clear",        &UCITranslator::_clearConsole},
        {     "clean",        &UCITranslator::_clearConsole},
        {       "cls",        &UCITranslator::_clearConsole},
        { "ctpm", &UCITranslator::_calculateTimePerMove }, // Calculate time per move
    };

    std::string workStr;
    size_t pos = 0;

    while ((pos = ParseTools::ExtractNextWord(buffer, workStr, pos)) != ParseTools::InvalidNextWorldRead)
        if (auto iter = CommandBuff.find(workStr); iter != CommandBuff.end())
        {
            auto commandType = (this->*(iter->second))(buffer.substr(pos));

            // Check whether some joining could be done between parsing commands
            if (commandType != UCICommand::goCommand && !_engine.TManager.IsSearchOn())
                _engine.TManager.Consolidate();

            return commandType;
        }
    return UCICommand::InvalidCommand;
}

UCITranslator::UCICommand UCITranslator::_stopResponse([[maybe_unused]] const std::string &)
{
    _engine.StopSearch();
    return UCICommand::stopCommand;
}

UCITranslator::UCICommand UCITranslator::_goResponse(const std::string &str)
{
    // Known subcommands of "go" command
    static std::unordered_map<std::string, UCICommand (UCITranslator::*)(const std::string &, size_t)> commands{
        {     "perft",       &UCITranslator::_goPerftResponse},
        {     "debug",       &UCITranslator::_goDebugResponse},
        { "deepDebug",   &UCITranslator::_goDeepDebugResponse},
        {      "file",        &UCITranslator::_goFileResponse},
        {  "perfComp",    &UCITranslator::_goPerfCompResponse},
        {"searchPerf", &UCITranslator::_goSearchPerftResponse},
    };

    std::string workStr;
    if (ParseTools::InvalidNextWorldRead == ParseTools::ExtractNextWord(str, workStr, 0))
        return UCICommand::InvalidCommand;

    if (auto iter = commands.find(workStr); iter != commands.end())
        // If subcommand is recognized, call it
        return (this->*(iter->second))(str, workStr.size() + 1);
    else
        // Otherwise, perform regular search command path
        return _goSearchRegular(str);
}

UCITranslator::UCICommand UCITranslator::_positionResponse(const std::string &str)
{
    std::string workStr;

    // Extract next token
    size_t pos = ParseTools::ExtractNextWord(str, workStr, 0);
    if (pos == ParseTools::InvalidNextWorldRead)
        return UCICommand::InvalidCommand;

    // find position of "moves" token
    const size_t movesCord = str.find("moves", pos);

    if (workStr == "fen")
    {
        // If there is no moves token, then assumes that rest of the string is fen position. Otherwise, extract only
        // part [] between "fen [] moves ..."
        _fenPosition = movesCord == std::string::npos ? ParseTools::GetTrimmed(str.substr(pos))
                                                      : ParseTools::GetTrimmed(str.substr(pos, movesCord - pos));

        // Load position
        _engine.SetFenPosition(_fenPosition);
    }
    else if (workStr == "startpos")
        // Reset the position inside the engine
        _engine.SetStartPos();
    else
        return UCICommand::InvalidCommand;

    // If there are moves token, then apply them
    if (movesCord != std::string::npos)
    {
        // shift by moves length
        pos = movesCord + 5;

        // Parse UCI encoded moves
        const std::vector<std::string> movesVect = ParseTools::Split(str, pos);

        if (!_engine.ApplyMoves(movesVect))
            return UCICommand::InvalidCommand;

        _appliedMoves = movesVect;
    }
    else
        _appliedMoves.clear();

    return UCICommand::positionCommand;
}

UCITranslator::UCICommand UCITranslator::_ucinewgameResponse([[maybe_unused]] const std::string &)
{
    _engine.RestartEngine();
    _appliedMoves.clear();
    return UCICommand::ucinewgameCommand;
}

UCITranslator::UCICommand UCITranslator::_setoptionResponse(const std::string &str)
{
    std::string workStr;

    // Read next token and expect it to be "name"
    size_t pos = ParseTools::ExtractNextWord(str, workStr, 0);
    if (pos == ParseTools::InvalidNextWorldRead || workStr != "name")
        return UCICommand::InvalidCommand;

    // Read tokens until "value" token is found or line is fully processed
    std::string optionName{};
    while ((pos = ParseTools::ExtractNextWord(str, workStr, pos)) != ParseTools::InvalidNextWorldRead &&
           workStr != "value")
    {
        optionName += workStr + ' ';
    }

    // removing last added space
    if (!optionName.empty())
        optionName.pop_back();

    // Check whether option is valid
    if (!Engine::GetEngineInfo().options.contains(optionName))
    {
        WrapTraceMsgError(std::format("Option {} not found.", optionName));
        return UCICommand::InvalidCommand;
    }

    // If there is option value given parse it and process it -> then report the outcome
    std::string arg = workStr != "value" ? std::string() : ParseTools::GetTrimmed(str.substr(pos));
    if (Engine::GetEngineInfo().options.at(optionName)->TryChangeValue(arg, _engine))
        return UCICommand::setoptionCommand;

    WrapTraceMsgError(std::format("Option {} refused to process the value: {}", optionName, arg));
    return UCICommand::InvalidCommand;
}

UCITranslator::UCICommand UCITranslator::_uciResponse([[maybe_unused]] const std::string &unused)
{
    GlobalLogger.LogStream << "id name " << Engine::GetEngineInfo().name << '\n';
    GlobalLogger.LogStream << "id author " << Engine::GetEngineInfo().author << '\n';

    for (const auto &opt : Engine::GetEngineInfo().options)
    {
        GlobalLogger.LogStream << *opt.second;
    }
    GlobalLogger.LogStream << "uciok" << std::endl;
    return UCICommand::uciCommand;
}

UCITranslator::UCICommand UCITranslator::_isReadyResponse([[maybe_unused]] const std::string &unused)
{
    GlobalLogger.LogStream << "readyok" << std::endl;
    return UCICommand::isreadyCommand;
}

UCITranslator::UCICommand UCITranslator::_displayResponse([[maybe_unused]] const std::string &unused)
{
    _engine.WriteBoard();
    return UCICommand::displayCommand;
}

UCITranslator::UCICommand UCITranslator::_displayHelpResponse([[maybe_unused]] const std::string &unused)
{
    static auto CustomCommands =
        "In addition to standard UCI commands, these are implemented:\n"
        "- go perft \"depth\" - Simple PERFT test.\n"
        "- go debug \"depth\" - debugging tool reporting first occured error in comparison to any engine\n"
        "                which implements \"go perft command\" - default target engine is stockfish\n"
        "- go deepDebug \"depth\" - debugging tool, which is used to possibly identify invalid move chains which "
        "produces\n"
        "                 buggy result.\n"
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
        "   - \"go searchPerf\" - will run tests on searchTests.csv\n";

    GlobalLogger.LogStream << "Help content:\n\n"
                           << "TODO MAIN HELP\n\n" // TODO
                           << CustomCommands;

    return UCICommand::helpCommand;
}

UCITranslator::UCICommand UCITranslator::_quitResponse([[maybe_unused]] const std::string &unused)
{
    return UCICommand::quitCommand;
}

UCITranslator::UCICommand UCITranslator::_clearConsole([[maybe_unused]] const std::string &unused)
{
#ifdef __unix__
    system("clear");
#elif defined(__WIN32__)
    system("cls");
#endif

    return UCICommand::displayCommand;
}

UCITranslator::UCICommand UCITranslator::_displayFenResponse([[maybe_unused]] const std::string &unused)
{
    GlobalLogger.LogStream << "Acquired fen translation:\n" << _engine.GetFenTranslation() << '\n';
    return UCICommand::displayCommand;
}

UCITranslator::UCICommand UCITranslator::_goPerftResponse(const std::string &str, size_t pos)
{
    int depth;
    if (_intParser(str, pos, depth) == ParseTools::InvalidNextWorldRead)
        return UCICommand::InvalidCommand;

    _engine.GoPerft(depth);
    return UCICommand::goCommand;
}

UCITranslator::UCICommand UCITranslator::_goDebugResponse(const std::string &str, size_t pos)
{
    int depth;
    if (_intParser(str, pos, depth) == ParseTools::InvalidNextWorldRead)
        return UCICommand::InvalidCommand;

    const MoveGenerationTester tester{Engine::GetDebugEnginePath()};
    [[maybe_unused]] auto unused = tester.PerformSingleShallowTest(_fenPosition, depth, _appliedMoves, true);
    return UCICommand::goCommand;
}

UCITranslator::UCICommand UCITranslator::_goDeepDebugResponse(const std::string &str, size_t pos)
{
    int depth;
    if (_intParser(str, pos, depth) == ParseTools::InvalidNextWorldRead)
        return UCICommand::InvalidCommand;

    const MoveGenerationTester tester{Engine::GetDebugEnginePath()};
    tester.PerformDeepTest(_fenPosition, depth, _appliedMoves);
    return UCICommand::goCommand;
}

UCITranslator::UCICommand UCITranslator::_goFileResponse(const std::string &str, size_t pos)
{
    std::string path{};
    ParseTools::ExtractNextWord(str, path, pos);
    const MoveGenerationTester tester{Engine::GetDebugEnginePath()};
    const bool result = tester.PerformSeriesOfDeepTestFromFile(path);

    if (!result)
        return UCICommand::InvalidCommand;
    return UCICommand::goCommand;
}

UCITranslator::UCICommand UCITranslator::_goPerfCompResponse(const std::string &str, size_t pos)
{
    std::string file1Str{};
    std::string file2Str{};
    pos = ParseTools::ExtractNextWord(str, file1Str, pos);
    if (pos != ParseTools::InvalidNextWorldRead)
        ParseTools::ExtractNextWord(str, file2Str, pos);

    const MoveGenerationTester tester{Engine::GetDebugEnginePath()};
    bool result = tester.PerformPerformanceTest(file1Str, file2Str);
    if (!result)
        return UCICommand::InvalidCommand;
    return UCICommand::goCommand;
}

UCITranslator::UCICommand UCITranslator::_goSearchPerftResponse(const std::string &str, size_t pos)
{
    std::string file1Str{};
    std::string file2Str{};
    pos = ParseTools::ExtractNextWord(str, file1Str, pos);
    if (pos != ParseTools::InvalidNextWorldRead)
        ParseTools::ExtractNextWord(str, file2Str, pos);

    bool result = SearchPerfTester::PerformSearchPerfTest(file1Str, file2Str, _engine.TManager.GetDefaultStack());
    if (!result)
        return UCICommand::InvalidCommand;
    return UCICommand::goCommand;
}

UCITranslator::UCICommand UCITranslator::_goSearchRegular(const std::string &str)
{
    // all known parameter tokens for go command
    static std::unordered_map<std::string, size_t (*)(const std::string &, size_t, GoInfo &)> params{
        {"movetime", &_goMoveTimeResponse},
        {    "binc", &_goBIncTimeResponse},
        {    "winc", &_goWIncTimeResponse},
        {   "btime",    &_goBTimeResponse},
        {   "wtime",    &_goWTimeResponse},
        {   "depth",    &_goDepthResponse},
    };

    GoInfo info{};
    std::string workStr{};
    size_t pos = 0;

    // Parse all parameters
    while ((pos = ParseTools::ExtractNextWord(str, workStr, pos)) != ParseTools::InvalidNextWorldRead &&
           workStr != "infinite")
    {
        // Check if parameter is valid
        if (auto iter = params.find(workStr); iter != params.end())
        {
            pos = (*(iter->second))(str, pos, info);

            // Check whether process was successful
            if (pos == ParseTools::InvalidNextWorldRead)
                return UCICommand::InvalidCommand;
        }
        else
            // if not simply stop parsing and proceed to performing search
            break;
    }

    // in case infinite parameter was passed there is special case call
    if (workStr == "infinite")
        _engine.GoInfinite();
    else
    // otherwise perform validation and if succeeded perform search
    {
        if (info.depth == GoInfo::NotSet && info.timeInfo.moveTime == GoTimeInfo::NotSet &&
            !info.timeInfo.IsColorTimeSet(_engine.GetMovingColor()))
            return UCICommand::InvalidCommand;

        // After passing this checks validation is complete,
        // all the default parameters are chosen to be able to perform fully valid search.
        _engine.Go(info, _appliedMoves);
    }
    return UCICommand::goCommand;
}

size_t UCITranslator::_goBIncTimeResponse(const std::string &str, const size_t pos, GoInfo &info)
{
    return _msTimeParser(str, pos, info.timeInfo.bInc);
}

size_t UCITranslator::_goMoveTimeResponse(const std::string &str, const size_t pos, GoInfo &info)
{
    return _msTimeParser(str, pos, info.timeInfo.moveTime);
}

size_t UCITranslator::_goWIncTimeResponse(const std::string &str, const size_t pos, GoInfo &info)
{
    return _msTimeParser(str, pos, info.timeInfo.wInc);
}

size_t UCITranslator::_goBTimeResponse(const std::string &str, const size_t pos, GoInfo &info)
{
    return _msTimeParser(str, pos, info.timeInfo.bTime);
}

size_t UCITranslator::_goWTimeResponse(const std::string &str, const size_t pos, GoInfo &info)
{
    return _msTimeParser(str, pos, info.timeInfo.wTime);
}

size_t UCITranslator::_goDepthResponse(const std::string &str, size_t pos, GoInfo &info)
{
    return _intParser(str, pos, info.depth);
}

size_t UCITranslator::_intParser(const std::string &str, size_t pos, int &out)
{
    static constexpr auto convert = [](const std::string &str) -> int
    {
        return std::stoi(str);
    };
    return ParseTools::ExtractNextNumeric<int, convert>(str, pos, out);
}

size_t UCITranslator::_msTimeParser(const std::string &str, size_t pos, lli &out)
{
    static constexpr auto convert = [](const std::string &str) -> lli
    {
        return std::stoll(str);
    };
    pos = ParseTools::ExtractNextNumeric<lli, convert>(str, pos, out);

    return out < 1 ? ParseTools::InvalidNextWorldRead : pos;
}

UCITranslator::UCICommand UCITranslator::_calculateTimePerMove(const std::string &str) {
    static FileLogger timePerMoveLogger("timePerMove.log");

    static std::unordered_map<std::string, size_t (*)(const std::string &, size_t, lli &)> params{
            {"movetime", &_msTimeParser},
            {    "binc", &_msTimeParser},
            {    "winc", &_msTimeParser},
            {   "btime",    &_msTimeParser},
            {   "wtime",    &_msTimeParser},
    };

    GoTimeInfo info{};
    std::string workStr{};
    size_t pos = 0;

    std::unordered_map<std::string, lli*> infoArgs{
        {"wtime", &info.wTime},
        {"btime", &info.bTime},
        {"winc", &info.wInc},
        {"binc", &info.bInc},
        {"movetime", &info.moveTime},
    };

    // Parse all parameters
    while ((pos = ParseTools::ExtractNextWord(str, workStr, pos)) != ParseTools::InvalidNextWorldRead)
    {
        // Check if parameter is valid
        if (auto iter = params.find(workStr); iter != params.end())
        {
            pos = (*(iter->second))(str, pos, *infoArgs[workStr]);

            // Check whether process was successful
            if (pos == ParseTools::InvalidNextWorldRead)
                return UCICommand::InvalidCommand;
        }
        else
            // if not simply stop parsing and proceed to performing search
            break;
    }

    Board board = _engine.GetUnderlyingBoardCopy();
    uint16_t age = _engine.GetAge();

    auto [ timeLimitClockMs, timeLimitPerMoveMs, incrementMs ] = GameTimeManagerUtils::ParseGoTimeInfo(info, (Color)_engine.GetMovingColor());
    if (timeLimitClockMs == GoTimeInfo::Infinite && timeLimitPerMoveMs == GoTimeInfo::Infinite)
        return UCICommand::InvalidCommand;

    GlobalLogger.TraceStream << "[ INFO ] Time limit clock: " << timeLimitClockMs << std::endl;
    GlobalLogger.TraceStream << "[ INFO ] Time limit per move: " << timeLimitPerMoveMs << std::endl;
    GlobalLogger.TraceStream << "[ INFO ] Increment: " << incrementMs << std::endl;

    lli timePerMove = GameTimeManager::CalculateTimeMsPerMove(board, timeLimitClockMs, timeLimitPerMoveMs, incrementMs, age);

    GlobalLogger.LogStream << "Calculated time per move:" << timePerMove << std::endl;
    timePerMoveLogger.LogStream << timePerMove << std::endl;

    return UCITranslator::UCICommand::debugCommand;
}
