//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef UCITRANSLATOR_H
#define UCITRANSLATOR_H

#include "../Engine.h"

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

    std::string _fenPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Engine& engine;
};

#endif //UCITRANSLATOR_H