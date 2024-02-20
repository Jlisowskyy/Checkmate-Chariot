//
// Created by Jlisowskyy on 2/16/24.
//

#ifndef PARSETOOLS_H
#define PARSETOOLS_H

#include <string>

#include "compilationConstants.h"

struct ParseTools {
    // ------------------------------
    // Class creation
    // ------------------------------

    ParseTools() = default;
    ~ParseTools() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // Parses passed string 'str' splitting string into words one by one startring by given position pos.
    // returns position after the end of parsed word.
    // If returned position is equal to 0, then no word was detected.
    static size_t ExtractNextWord(const std::string& str, std::string& wordOut, size_t startPos);

    // Returns last character present in outBuffer + 1
    static size_t ExtractNextLine(size_t startPos, size_t maxPos, const char* inBuffer, std::string& outBuffer);
    static lli ParseTolli(const std::string& str);
    static size_t TrimLeft(const std::string& str);
    static size_t TrimRight(const std::string& str);
    static std::string GetTrimmed(const std::string& str);


    // ------------------------------
    // Class fields
    // ------------------------------

};

#endif //PARSETOOLS_H