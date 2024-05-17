//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef UCITRANSLATOR_H
#define UCITRANSLATOR_H

#include "../Engine.h"
#include "../ThreadManagement/GameTimeManager.h"

/*
 *                  ADDITIONAL NOTES
 *
 *  In addition to standard UCI commands, these are implemented:
 *  - go perft "depth" - Simple PERFT test.
 *  - go debug "depth" - debugging tool reporting first occured error in comparison to any engine
 *                       which implements "go perft command" - default target engine is stockfish
 *  - go deepDebug "depth" - debugging tool, which is used to possibly identify invalid move chains which produces
 *                       buggy result.
 *  - go fullDebug "depth" - traverses whole tree and invokes simple debug test on each leaf parent to check
 *                       move correctness on lowest level possible. Incredibly slow - use only for lower search. Could
 * be optimised.
 *  - fen - simply displays fen encoding of current map
 *  - go perfComp "input file" "output file" - generates csv file to "output file" which contains information
 *                       about results of simple comparison tests, which uses external engine times to get results
 *  - go file "input file" - performs series of deepDebug on each position saved inside input file. For simplicity
 *                       "input file" must be containing csv records in the given manner: "fen position", "depth"
 *
 *  Where "depth" is integer value indicating layers of a traversed move tree.
 */

/*
 * This class is used as an interface between UCI commands and engine.
 * Parses, translates and applies commands to the engine.
 *
 * */

class UCITranslator
{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    public:
    /* takes a reference to engine instance on which it will operate */
    UCITranslator(Engine &engine) : _engine(engine) { _engine.Initialize(); }

    // ------------------------------
    // internal types
    // ------------------------------

    /* all known command tokens */
    enum class UCICommand
    {
        InvalidCommand,
        uciCommand,
        isreadyCommand,
        setoptionCommand,
        ucinewgameCommand,
        positionCommand,
        goCommand,
        stopCommand,
        quitCommand,
        displayCommand,
        helpCommand,
        debugCommand,
        ponderhitCommand,
    };

    // ------------------------------
    // Clas interaction
    // ------------------------------

    /*
     * Method parses input from the stream until it receives EOF or quit command.
     * Returns last processed command to distinguish between quit and EOF.
     * */

    UCICommand BeginCommandTranslation(std::istream &input);

    // ------------------------------
    // private methods
    // ------------------------------

    private:
    /*
     * Method simply, according to uci spec, parses line until it recognizes command token
     * and dispatchers it to corresponding procedure.
     *
     * Returns Invalid command when error occurred. Otherwise, returns command token of processed command.
     * */

    [[nodiscard]] UCICommand _dispatchCommands(const std::string &buffer);

    // ------------------------------
    // Command response methods
    // ------------------------------

    UCICommand _calculateTimePerMove(const std::string &unused);

    /* "stop" implementation */
    UCICommand _stopResponse([[maybe_unused]] const std::string &unused);

    /* "go" uci command response, simply dispatches processing into subcommand specific functions
     *
     * "go" implementation
     * */
    [[nodiscard]] UCICommand _goResponse(const std::string &str);

    /* method fully processes  fen and startpos subcommand
     *
     * "position" implementation
     * */
    [[nodiscard]] UCICommand _positionResponse(const std::string &str);

    /*
     *  Method resets engine state and clears all applied moves.
     *
     *  Performing actions:
     *  - Cleaning Transposition table
     *  - Resetting board
     *  - Clearing applied moves
     *
     * */

    UCICommand _searchZobrist(const std::string &str);

    UCICommand _ucinewgameResponse([[maybe_unused]] const std::string &);

    /* Function simply parses option and tries to apply it on engine.
     *
     * "setoption" implementation
     * */
    [[nodiscard]] UCICommand _setoptionResponse(const std::string &str);

    /* "uci" implementation */
    UCICommand _uciResponse([[maybe_unused]] const std::string &);

    /* "isready" implementation */
    UCICommand _isReadyResponse([[maybe_unused]] const std::string &);

    /* "ponderhit" implementation */
    UCICommand _ponderhitResponse([[maybe_unused]] const std::string &)
    {
        _engine.PonderHit();
        return UCICommand::ponderhitCommand;
    }

    /* Own added command to display the board */
    UCICommand _displayResponse([[maybe_unused]] const std::string &);

    /* Own added command to display the fen encoded position */
    UCICommand _displayFenResponse([[maybe_unused]] const std::string &);

    /* Own added command to display the help */
    UCICommand _displayHelpResponse([[maybe_unused]] const std::string &);

    /* "quit" implementation - simply returns quit token */
    UCICommand _quitResponse([[maybe_unused]] const std::string &);

    /* Own added command to clear the console */
    UCICommand _clearConsole([[maybe_unused]] const std::string &);

    /* Simply start perft move */
    UCICommand _goPerftResponse(const std::string &str, size_t pos);

    /* "go ponder" implementation */
    UCICommand _goPonder(const std::string &str, size_t pos);

    /* Simply starts debug test - refer to MoveGenerationTester.PerformSingleShallowTest */
    UCICommand _goDebugResponse(const std::string &str, size_t pos);

    /* Simply starts deep debug test - refer to MoveGenerationTester.PerformDeepTest */
    UCICommand _goDeepDebugResponse(const std::string &str, size_t pos);

    /* Simply starts deep series of debug test - refer to MoveGenerationTester.PerformSeriesOfDeepTestFromFile */
    UCICommand _goFileResponse(const std::string &str, size_t pos);

    /* Simply starts deep debug test - refer to MoveGenerationTester.PerformPerformanceTest */
    UCICommand _goPerfCompResponse(const std::string &str, size_t pos);

    /* Simply start search Perft test - refer to MoveGenerationTester.PerformSearchPerfTest */
    UCICommand _goSearchPerftResponse(const std::string &str, size_t pos);

    /* Regular uci go command */
    UCICommand _goSearchRegular(const std::string &str);

    // ------------------------------
    /* Go parameter parsing and checking functions */

    static size_t _goMoveTimeResponse(const std::string &str, size_t pos, GoInfo &info);

    static size_t _goBIncTimeResponse(const std::string &str, size_t pos, GoInfo &info);

    static size_t _goWIncTimeResponse(const std::string &str, size_t pos, GoInfo &info);

    static size_t _goBTimeResponse(const std::string &str, size_t pos, GoInfo &info);

    static size_t _goWTimeResponse(const std::string &str, size_t pos, GoInfo &info);

    static size_t _goDepthResponse(const std::string &str, size_t pos, GoInfo &info);

    static size_t goPonderResponse(const std::string &str, size_t pos, GoInfo &info);

    // ------------------------------

    /* Method simply parses int from the 'str' starting on position 'pos', places the result in 'out'.
     *
     * Returns position of the first character after the parsed int or InvalidToken when parsing is not possible.
     * */
    static size_t _intParser(const std::string &str, size_t pos, int &out);

    /* Method simply parses time given in ms (checking if return >= 1) from the 'str' starting on position 'pos', places
     * the result in 'out'.
     *
     * Returns position of the first character after the parsed int or InvalidToken when parsing is not possible.
     * */
    static size_t _msTimeParser(const std::string &str, size_t pos, lli &out);

    // ------------------------------
    // private fields
    // ------------------------------

    std::vector<std::string> _appliedMoves{};
    std::string _fenPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Engine &_engine;
};

#endif // UCITRANSLATOR_H
