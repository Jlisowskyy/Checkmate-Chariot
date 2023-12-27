//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef UCITRANSLATOR_H
#define UCITRANSLATOR_H

#include <iostream>
#include <variant>

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
            if (workStr == "quit" || workStr == "exit")
                return UCICommand::quitCommand;
        }

        return UCICommand::InvalidCommand;
    }

    UCICommand _stopResponse() const {
        engine.StopSearch();
        return  UCICommand::stopCommand;
    }

    [[nodiscard]] UCICommand _goResponse(const std::string& str) const{
        std::string workStr;
        size_t pos = _genNextWord(str, workStr, 0);
        if (pos == 0) return UCICommand::InvalidCommand;

        if (workStr == "perft")
            engine.GoPerft();
        else if (workStr == "infinite")
            engine.GoInfinite();
        else if (workStr == "depth") {
            pos = _genNextWord(str, workStr, pos);
            if (pos == 0) return UCICommand::InvalidCommand;
            const lli arg = _parseTolli(workStr);
            if (arg <= 0) return UCICommand::InvalidCommand;

            engine.GoMovetime(arg);
        }
        else if (workStr == "movetime"){
            pos = _genNextWord(str, workStr, pos);
            if (pos == 0) return UCICommand::InvalidCommand;
            const lli arg = _parseTolli(workStr);
            if (arg <= 0) return UCICommand::InvalidCommand;

            engine.GoMovetime(arg);
        }

        return UCICommand::goCommand;
    }

    [[nodiscard]] UCICommand _positionResponse(const std::string& str) {
        std::string workStr;
        size_t pos = _genNextWord(str, workStr, 0);
        if (pos == 0) return UCICommand::InvalidCommand;

        const size_t movesCord = str.find("moves", pos);

        if (workStr == "fen") {
            const std::string fenPos = movesCord == std::string::npos ?
                _getTrimmed(str.substr(pos)) : _getTrimmed(str.substr(pos, movesCord - pos));

            engine.SetFenPosition(fenPos);
        }
        else if (workStr != "startpos")
            return UCICommand::InvalidCommand;

        if (movesCord != std::string::npos) {
            pos = movesCord + 5;

            std::vector<std::string> movesVect{};
            while((pos = _genNextWord(str, workStr, pos)) != 0) {
                movesVect.push_back(workStr);
            }

            if (!engine.ApplyMoves(movesVect))
                return UCICommand::InvalidCommand;
        }

        return UCICommand::positionCommand;
    }

    UCICommand _ucinewgameResponse() const {
        engine.RestartEngine();
        return UCICommand::ucinewgameCommand;
    }

    [[nodiscard]] UCICommand _setoptionResponse(const std::string& str) const {
        std::string workStr;
        size_t pos = _genNextWord(str, workStr, 0);
        if (pos == 0 || workStr != "name") return UCICommand::InvalidCommand;

        std::string optionName{};
        while((pos = _genNextWord(str, workStr, pos)) != 0 && workStr != "value") {
            optionName += workStr + ' ';
        }

        // space cleaning
        if (!optionName.empty()) optionName.pop_back();

        // TODO: Consider error mesage here - unrecognized option
        if (!Engine::GetEngineInfo().options.contains(optionName))
            return UCICommand::InvalidCommand;


        // argument option
        std::string arg = workStr != "value" ? std::string() : _getTrimmed(str.substr(pos));
        if (Engine::GetEngineInfo().options.at(optionName)->TryChangeValue(arg, engine))
            return UCICommand::setoptionCommand;
        return UCICommand::InvalidCommand;
    }

    static UCICommand _uciResponse() {
        std::cout << "id name " << Engine::GetEngineInfo().name << '\n';
        std::cout << "id author " << Engine::GetEngineInfo().author << '\n';

        for(const auto& opt: Engine::GetEngineInfo().options) {
            std::cout << *opt.second;
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
        while(pos < str.length() && !isblank(str[pos])) { ++pos; }
        const size_t end = pos;

        if (beg == end) return 0;

        wordOut = str.substr(beg, end-beg);
        return end;
    }

    static lli _parseTolli(const std::string& str) {
        errno = 0;
        return strtoll(str.c_str(), nullptr, 10);
    }

    static size_t _trimLeft(const std::string& str){
        size_t ind = 0;
        while(ind < str.length() && std::isblank(str[ind])) { ++ind; }
        return ind;
    }

    static size_t _trimRight(const std::string& str) {
        size_t ind = str.length();
        while(ind > 0 && std::isblank(str[ind])) { -- ind; }
        return ind;
    }

    static std::string _getTrimmed(const std::string& str) {
        const size_t tLeft = _trimLeft(str);
        const size_t tRight = _trimRight(str);

        if (tLeft > tRight) return "";

        return str.substr(tLeft, tRight - tLeft);
    }

    // ------------------------------
    // private fields
    // ------------------------------

    Engine& engine;
};

#endif //UCITRANSLATOR_H