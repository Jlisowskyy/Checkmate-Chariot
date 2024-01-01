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

    void BeginCommandTranslation() const;

    // ------------------------------
    // private methods
    // ------------------------------
private:

    [[nodiscard]] UCICommand _cleanMessage(const std::string& buffer) const;
    UCICommand _stopResponse() const;
    [[nodiscard]] UCICommand _goResponse(const std::string& str) const;
    [[nodiscard]] UCICommand _positionResponse(const std::string& str) const;
    UCICommand _ucinewgameResponse() const;
    [[nodiscard]] UCICommand _setoptionResponse(const std::string& str) const;
    static UCICommand _uciResponse();
    UCICommand static _isReadyResponse();
    UCICommand _displayResponse() const;

    static size_t _genNextWord(const std::string& str, std::string& wordOut, size_t pos)
        // Parses passed string 'str' splitting string into words one by one startring by given position pos.
        // returns position after the end of parsed word.
        // If returned position is equal to 0, then no word was detected.
    ;

    static lli _parseTolli(const std::string& str);
    static size_t _trimLeft(const std::string& str);
    static size_t _trimRight(const std::string& str);
    static std::string _getTrimmed(const std::string& str);

    // ------------------------------
    // private fields
    // ------------------------------

    Engine& engine;
};

#endif //UCITRANSLATOR_H