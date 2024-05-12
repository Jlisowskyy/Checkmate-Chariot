//
// Created by Jlisowskyy on 2/26/24.
//

#ifndef OPENINGBOOK_H
#define OPENINGBOOK_H

#include "MoveTRIE.h"
#include "../Interface/Logger.h"

#include <string>

struct OpeningBook
{
    // ------------------------------
    // Class inner Types
    // ------------------------------

    // used to choose which type passed file is
    enum class bookFileType
    {
        binary,
        text_uci
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    OpeningBook() = default;

    // initializing the only const field
    OpeningBook(const std::string &bookPath, bookFileType type);

    // ------------------------------
    // Class interaction
    // ------------------------------

    void LoadBook(const std::string &bookPath, bookFileType type){
        if (_isCorrectlyLoaded)
        {
            WrapTraceMsgError("Opening book was already loaded, skipping the loading process");
            return;
        }

        try
        {
            switch (type)
            {
            case bookFileType::binary:
                _readBinaryBook(bookPath);
                break;
            case bookFileType::text_uci:
                _readTextBook(bookPath);
                break;
            }

            WrapTraceMsgInfo("Book correctly loaded!");
            _isCorrectlyLoaded = true;
        }
        catch (const std::exception &exc)
        {
            GlobalLogger.LogStream << std::format(
                "[ ERROR ] Book with path: {} was not correctly loaded due to following fact:\n\t{}\n", bookPath, exc.what()
            );
            _isCorrectlyLoaded = false;
        }
    }

    // Possibly in use when array representation will be considered
    void SaveToBinary(const std::string &outputPath) const;
    [[nodiscard]] bool IsLoadedCorrectly() const;

    [[nodiscard]] const std::string &GetRandomNextMove(const std::vector<std::string> &moves) const;
    [[nodiscard]] const std::vector<std::string> &GetAllPossibleNextMoves(const std::vector<std::string> &moves) const;

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    void _readBinaryBook(const std::string &bookPath)
        // Possibly in use when array representation will be considered
        ;

    void _readTextBook(const std::string &bookPath);

    // ------------------------------
    // Class fields
    // ------------------------------

    MoveTRIE _moveTrie{};
    bool _isCorrectlyLoaded{};
};

#endif // OPENINGBOOK_H
