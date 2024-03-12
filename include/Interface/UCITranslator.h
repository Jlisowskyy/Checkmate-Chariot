//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef UCITRANSLATOR_H
#define UCITRANSLATOR_H

#include "../Engine.h"

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
 *                       move correctnes on lowest level possible. Insanly slow - use only for lower search. Could be
 * optimised.
 *  - fen - simply displays fen encoding of current map
 *  - go perfComp "input file" "output file" - generates csv file to "output file" which contains information
 *                       about results of simple comparison tests, which uses external engine times to get results
 *  - go file "input file" - performs series of deepDebug on each positions saved inside input file. For simplicity
 *                       "input file" must be containg csv records in given manner: "fen position", "depth"
 *
 *  Where "depth" is integer value indicating layers of traversed move tree.
 */

class UCITranslator
{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------
   public:
    UCITranslator(Engine& engine) : _engine(engine) {}

    // ------------------------------
    // internal types
    // ------------------------------

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
    };

    // ------------------------------
    // Clas interaction
    // ------------------------------

    UCICommand BeginCommandTranslation(std::istream& input);

    // ------------------------------
    // private methods
    // ------------------------------
   private:
    [[nodiscard]] UCICommand _cleanMessage(const std::string& buffer);

    // ------------------------------
    // Command response methods
    // ------------------------------

    UCICommand _stopResponse([[maybe_unused]] const std::string& unused);

    [[nodiscard]] UCICommand _goResponse(const std::string& str);

    [[nodiscard]] UCICommand _positionResponse(const std::string& str);

    UCICommand _ucinewgameResponse([[maybe_unused]] const std::string& unused);

    [[nodiscard]] UCICommand _setoptionResponse(const std::string& str);

    UCICommand _uciResponse([[maybe_unused]] const std::string& unused);

    UCICommand _isReadyResponse([[maybe_unused]] const std::string& unused);

    UCICommand _displayResponse([[maybe_unused]] const std::string& unused);

    UCICommand _displayFenResponse([[maybe_unused]] const std::string& unused);

    UCICommand _displayHelpResponse([[maybe_unused]] const std::string& unused);

    UCICommand _quitResponse([[maybe_unused]] const std::string& unused);

    UCICommand _clearConsole([[maybe_unused]] const std::string& unused);

    // ------------------------------
    // private fields
    // ------------------------------

    std::vector<std::string> _appliedMoves{};
    std::string _fenPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Engine& _engine;
};

#endif  // UCITRANSLATOR_H
