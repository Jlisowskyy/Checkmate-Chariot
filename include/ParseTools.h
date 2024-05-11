//
// Created by Jlisowskyy on 2/16/24.
//

#ifndef PARSETOOLS_H
#define PARSETOOLS_H

#include <fstream>
#include <string>
#include <vector>

#include "CompilationConstants.h"

/*
 * Class gathers various tools and utilities used to parse strings.
 * */

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

    // Parses first found word inside the passed string 'str', starting on given position 'startPos'.
    // It escapes all characters defined by 'crit' function.
    // returns position after the end of parsed word.
    // If the returned position is equal to InvalidNextWorldRead, then no word was detected.

    static constexpr size_t InvalidNextWorldRead = 0;

    template <int (*crit)(int) = isblank>
    static size_t ExtractNextWord(const std::string &str, std::string &wordOut, size_t startPos);

    /* Function parses next word with "ExtractNextWord" function and then converts it to the numeric type NumT
     * with given 'convert' function.
     *
     * Return InvalidNextWorldRead when no conversion was possible.
     * */
    template <typename NumT, NumT (*convert)(const std::string &), int (*crit)(int) = isblank>
    static size_t ExtractNextNumeric(const std::string &str, size_t startPos, NumT &out);

    // Parses next line from the buffer to the "outBuffer". Returns position after the end of the line or maxPos (if no new line was found)
    static size_t ExtractNextLine(size_t startPos, size_t maxPos, const char *inBuffer, std::string &outBuffer);

    // Function returns index of first non-blank character in the string or string length if no such character was found.
    static size_t TrimLeft(const std::string &str);

    // Function returns index of first non-blank character from the end of the string or 0 if no such character was found.
    static size_t TrimRight(const std::string &str);

    // Function returns string with all leading and trailing blanks removed.
    static std::string GetTrimmed(const std::string &str);

    // Function splits the text into words, using the 'crit' function to determine the word boundaries.
    template <int (*crit)(int) = isblank>
    [[nodiscard]] static std::vector<std::string> Split(const std::string &text, size_t pos = 0);

    // returns number of '\n' + 1 when stream is good to read otherwise returns -1
    static signed_size_t GetLineCountFromFile(std::fstream &stream);

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

template <typename NumT, NumT (*convert)(const std::string &), int (*crit)(int)>
size_t ParseTools::ExtractNextNumeric(const std::string &str, size_t startPos, NumT &out)
{
    std::string depthStr{};
    startPos = ParseTools::ExtractNextWord<crit>(str, depthStr, startPos);
    if (startPos == ParseTools::InvalidNextWorldRead)
        return ParseTools::InvalidNextWorldRead;

    NumT depth;
    try
    {
        depth = convert(depthStr);
    }
    catch (const std::exception &exc)
    {
        return ParseTools::InvalidNextWorldRead;
    }

    out = depth;
    return startPos;
}

#endif // PARSETOOLS_H
