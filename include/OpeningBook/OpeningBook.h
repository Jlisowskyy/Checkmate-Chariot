//
// Created by Jlisowskyy on 2/26/24.
//

#ifndef OPENINGBOOK_H
#define OPENINGBOOK_H

#include <string>

#include <random>
#include <chrono>

#include "MoveTRIE.h"

struct  OpeningBook
{
    // ------------------------------
    // Class inner Types
    // ------------------------------

    // used to chose which type passed file is
    enum class bookFileType
    {
        binary,
        text_uci
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    // initalizing the only const field
    OpeningBook(const std::string& bookPath, bookFileType type);

    // ------------------------------
    // Class interaction
    // ------------------------------

    // Possibly in use when array representation will be considered
    void SaveToBinary(const std::string&outputPath) const;
    [[nodiscard]] bool IsLoadedCorrectly() const;

    [[nodiscard]] const std::string& GetRandomNextMove(const std::vector<std::string>&moves) const;
    [[nodiscard]] const std::vector<std::string>& GetAllPossibleNextMoves(const std::vector<std::string>&moves) const;

    // ------------------------------
    // Private class methods
    // ------------------------------
private:

    void _readBinaryBook(const std::string&bookPath)
        // Possibly in use when array representation will be considered
    ;

    void _readTextBook(const std::string&bookPath);

    // ------------------------------
    // Class fields
    // ------------------------------

    MoveTRIE _moveTrie{};
    bool _isCorrectlyLoaded{};
};

#endif //OPENINGBOOK_H
