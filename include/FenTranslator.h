//
// Created by Jlisowskyy on 12/27/23.
//

#ifndef FENTRANSLATOR_H
#define FENTRANSLATOR_H

#include <string>
#include <exception>
#include <format>
#include <stdexcept>

#include "EngineTypeDefs.h"

struct FenTranslator {
    // ------------------------------
    // Class interaction
    // ------------------------------

    static Board Translate(const std::string& fenPos) {
        Board workBoard{};
        size_t pos = 0;

        pos = _skipBlanks(pos, fenPos);
        pos = _processPositions(workBoard, pos, fenPos);
        pos = _skipBlanks(pos, fenPos);

        return workBoard;
    }

    // ------------------------------
    // private methods
    // ------------------------------


    static size_t _processPositions(Board& bd, size_t pos, const std::string& fenPos) {
        int processedFields = 0;
        std::string posBuffer = "00";

        while(pos < fenPos.length() && !std::isblank(fenPos[pos])) {
            posBuffer[0] = processedFields % 8 + 'a';
            posBuffer[1] = '8' - (processedFields >> 3);

            const char val = fenPos[pos];
            if (val >= '1' && val <= '8')
                processedFields += val - '0';
            else if (val != '/'){
                _addFigure(posBuffer, val, bd);
                ++processedFields;
            }

            if (processedFields > Board::BoardFields)
                throw std::runtime_error(
                    std::format("[ ERROR ] Too much fields are used inside passed fen position!\n")
                );

            ++pos;
        }

        if (processedFields < Board::BoardFields)
            throw std::runtime_error(
                std::format("[ ERROR ] Not enugh fields are used inside passed fen position!\n")
            );

        return pos;
    }

    static void _addFigure(const std::string& pos, char fig, Board& bd) {
        const auto field = static_cast<uint64_t>(strFieldMap.at(pos));

        if (!figToDescMap.contains(fig))
            throw std::runtime_error(
            std::format("[ ERROR ] Encountered invalid character ({0})inside fen position description!\n", fig)
            );

        bd.boards[figToDescMap.at(fig)] |= field;
    }

    static size_t _skipBlanks(size_t pos, const std::string& fenPos) {
        while(pos < fenPos.length() && std::isblank(fenPos[pos])) { ++pos; }
        return pos;
    }

};

#endif //FENTRANSLATOR_H
