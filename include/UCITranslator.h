//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef UCITRANSLATOR_H
#define UCITRANSLATOR_H

#include <iostream>

#include "Engine.h"

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
    };

    // ------------------------------
    // Clas interaction
    // ------------------------------

    void BeginCommandTranslation() {
        auto lastCommand = UCICommand::InvalidCommand;

        while (lastCommand != UCICommand::quitCommand) {
            std::string recordBuffer;
            std::getline(std::cin, recordBuffer);
            lastCommand = _cleanMessage(recordBuffer);

            if (lastCommand == UCICommand::InvalidCommand)
                std::cout << "ERR\n";
        }
    }
    // ------------------------------
    // private methods
    // ------------------------------
private:

    [[nodiscard]] UCICommand _cleanMessage(const std::string& buffer) {
        std::string workStr;
        size_t pos = 0;

        while ((pos = _genNextWord(buffer, workStr, pos)) != 0) {
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
            if (workStr == "quit")
                return UCICommand::quitCommand;
        }

        return UCICommand::InvalidCommand;
    }

    [[nodiscard]] UCICommand _stopResponse() {
        return  UCICommand::stopCommand;
    }

    [[nodiscard]] UCICommand _goResponse(const std::string& str) {

        return UCICommand::goCommand;
    }

    [[nodiscard]] UCICommand _positionResponse(const std::string& str) {

        return UCICommand::positionCommand;
    }

    [[nodiscard]] UCICommand _ucinewgameResponse() {

        return UCICommand::ucinewgameCommand;
    }

    [[nodiscard]] UCICommand _setoptionResponse(const std::string& str) {

        return UCICommand::setoptionCommand;
    }

    UCICommand _uciResponse() const {
        std::cout << "id name " << Engine::GetEngineInfo().name << '\n';
        std::cout << "id author " << Engine::GetEngineInfo().author << '\n';

        for(const auto& opt: Engine::GetEngineInfo().options) {
            std::cout << opt.second;
        }
        std::cout << "uciok" << std::endl;
        return UCICommand::uciCommand;
    }

    UCICommand static _isReadyResponse() {
        std::cout << "readyok" << std::endl;
        return UCICommand::isreadyCommand;
    }

    static size_t _genNextWord(const std::string& str, std::string& wordOut, size_t pos)
        // Parses passed string 'str' splitting string into words one by one startring by given position pos.
        // returns position after the end of parsed word.
        // If returned position is equal to 0, then no word was detected.
    {
        while(pos < str.length() && isblank(str[pos])) { ++pos; }
        const size_t beg = pos;
        while(pos < str.length() && isalnum(str[pos])) { ++pos; }
        const size_t end = pos;

        if (beg == end) return 0;

        wordOut = str.substr(beg, end-beg);
        return end;
    }

    // ------------------------------
    // private fields
    // ------------------------------

    Engine& engine;
};



#endif //UCITRANSLATOR_H