//
// Created by Jlisowskyy on 2/16/24.
//

#ifndef PARSETOOLS_H
#define PARSETOOLS_H

#include <fstream>
#include <string>
#include <vector>

#include "CompilationConstants.h"

struct ParseTools
{
    // ------------------------------
    // Class creation
    // ------------------------------

    ParseTools() = default;

    ~ParseTools() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // Parses passed string 'str' splitting string into words one by one starting by given position pos.
    // returns position after the end of parsed word.
    // If the returned position is equal to 0, then no word was detected.

    template <int (*crit)(int) = isblank>
    static size_t ExtractNextWord(const std::string &str, std::string &wordOut, size_t startPos);

    // Returns last character present in outBuffer + 1
    static size_t ExtractNextLine(size_t startPos, size_t maxPos, const char *inBuffer, std::string &outBuffer);

    static lli ParseTolli(const std::string &str);

    static size_t TrimLeft(const std::string &str);

    static size_t TrimRight(const std::string &str);

    static std::string GetTrimmed(const std::string &str);

    template <int (*crit)(int) = isblank>
    [[nodiscard]] static std::vector<std::string> Split(const std::string &text, size_t pos = 0);

    static ssize_t GetLineCountFromFile(std::fstream &stream)
        // returns number of '\n' + 1 when stream is good to read otherwise returns -1
        ;

    // ------------------------------
    // Class fields
    // ------------------------------
};

template <int (*crit)(int)>
size_t ParseTools::ExtractNextWord(const std::string &str, std::string &wordOut, size_t startPos)
{
    while (startPos < str.length() && isblank(str[startPos]))
    {
        ++startPos;
    }
    const size_t beg = startPos;
    while (startPos < str.length() && !crit(str[startPos]))
    {
        ++startPos;
    }
    const size_t end = startPos;

    if (beg == end)
        return 0;

    wordOut = str.substr(beg, end - beg);
    return end;
}

template <int (*crit)(int)> std::vector<std::string> ParseTools::Split(const std::string &text, size_t pos)
{
    std::vector<std::string> splittedWords{};
    std::string wordBuffer{};

    while ((pos = ParseTools::ExtractNextWord<crit>(text, wordBuffer, pos)) != 0) splittedWords.push_back(wordBuffer);

    return splittedWords;
}

#endif // PARSETOOLS_H
