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
 *
 *  Where "depth" is integer value indicating layers of traversed move tree.
 */

class UCITranslator {
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------
public:
    UCITranslator(Engine& engine) : engine(engine) {}

    // ------------------------------
    // internal types
    // ------------------------------

    enum class UCICommand {
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
    };

    // ------------------------------
    // Clas interaction
    // ------------------------------

    void BeginCommandTranslation();

    // ------------------------------
    // private methods
    // ------------------------------
private:

    [[nodiscard]] UCICommand _cleanMessage(const std::string& buffer);
    UCICommand _stopResponse() const;
    [[nodiscard]] UCICommand _goResponse(const std::string& str) const;
    [[nodiscard]] UCICommand _positionResponse(const std::string& str);
    UCICommand _ucinewgameResponse() const;
    [[nodiscard]] UCICommand _setoptionResponse(const std::string& str) const;
    static UCICommand _uciResponse();
    UCICommand static _isReadyResponse();
    UCICommand _displayResponse() const;

    // ------------------------------
    // private fields
    // ------------------------------

    std::vector<std::string> _appliedMoves{};
    std::string _fenPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Engine& engine;
};

#endif //UCITRANSLATOR_H