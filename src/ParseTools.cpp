//
// Created by Jlisowskyy on 2/16/24.
//

#include "../include/ParseTools.h"

#include <numeric>

size_t
ParseTools::ExtractNextLine(const size_t startPos, const size_t maxPos, const char *inBuffer, std::string &outBuffer)
{
    for (size_t pos = startPos; pos < maxPos; ++pos)
    {
        outBuffer += inBuffer[pos];
        if (inBuffer[pos] == '\n')
            return pos + 1;
    }

    return maxPos;
}

size_t ParseTools::TrimLeft(const std::string &str)
{
    size_t ind = 0;
    while (ind < str.length() && std::isblank(str[ind]))
    {
        ++ind;
    }
    return ind;
}

size_t ParseTools::TrimRight(const std::string &str)
{
    size_t ind = str.length();
    while (ind > 0 && std::isblank(str[ind]))
    {
        --ind;
    }
    return ind;
}

std::string ParseTools::GetTrimmed(const std::string &str)
{
    const size_t tLeft  = TrimLeft(str);
    const size_t tRight = TrimRight(str);

    if (tLeft > tRight)
        return "";

    return str.substr(tLeft, tRight - tLeft);
}

signed_size_t ParseTools::GetLineCountFromFile(std::fstream &stream)
{
    if (stream.bad())
        return -1;
    if (stream.eof())
        stream.clear();

    const std::streampos oldPos = stream.tellg();
    stream.seekg(0);

    signed_size_t newLineCount{};
    for (char buff; stream.get(buff);) newLineCount += (buff == '\n');

    stream.clear();
    stream.seekg(oldPos);
    return newLineCount + 1;
}
