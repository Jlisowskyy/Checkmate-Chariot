//
// Created by Jlisowskyy on 3/26/24.
//

#ifndef FILEMAP_H
#define FILEMAP_H

#include "MoveGeneration.h"

#include <cassert>

struct FileMap
{
    // ------------------------------
    // Class creation
    // ------------------------------

    FileMap() = delete;
    ~FileMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    static uint64_t GetPlainFile(const int msbPos)
    {
        return _plainFiles[msbPos];
    }

    static uint64_t GetFatFile(const int msbPos)
    {
        return _fatFiles[msbPos];
    }

    static uint64_t GetFronFatFile(const int msbPos, const int col)
    {
        return _frontFiles[col][msbPos];
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    static constexpr std::array<uint64_t, Board::BoardFields> _plainFiles = []() constexpr
    {
        std::array<uint64_t, Board::BoardFields> rv{};

        for (int msb = 0; msb < Board::BoardFields; ++msb)
        {
            const int boardPos = ConvertToReversedPos(msb);
            const int startpos = boardPos % 8;
            rv[msb] = GenMask(startpos, Board::BoardFields, 8);
        }

        return rv;
    }();

    static constexpr std::array<uint64_t, Board::BoardFields> _fatFiles = []() constexpr
    {
        std::array<uint64_t, Board::BoardFields> rv{};

        for (int msb = 0; msb < Board::BoardFields; ++msb)
        {
            const int boardPos = ConvertToReversedPos(msb);
            const int startpos = boardPos % 8;
            const uint64_t leftMask = startpos > 0 ? GenMask(startpos-1, Board::BoardFields, 8) : 0;
            const uint64_t rightMask = startpos < 7 ? GenMask(startpos+1, Board::BoardFields, 8) : 0;

            rv[msb] = GenMask(startpos, Board::BoardFields, 8) | leftMask | rightMask;
        }

        return rv;
    }();

    static inline std::array<std::array<uint64_t, Board::BoardFields>, 2> _frontFiles = []()
    {
        static constexpr int offset = 8;
        std::array<std::array<uint64_t, Board::BoardFields>, 2> rv{};

        for (int col = 0; col < 2; ++col)
        {
            for (int msb = 0; msb < Board::BoardFields; ++msb)
            {
                const int boardPos = ConvertToReversedPos(msb);
                const int nFiledOff = col == WHITE ? offset : -offset;
                if (const int nextRow = boardPos + nFiledOff; nextRow < 0 || nextRow >= Board::BoardFields) continue;

                const int startpos = boardPos % offset;

                uint64_t leftMask{};
                uint64_t rightMask{};
                if (startpos > 0)
                    leftMask = col == WHITE ?
                        GenMask(boardPos-1 + offset, Board::BoardFields, offset) :
                        GenMask(startpos-1, boardPos-1, offset);

                if (startpos < 7)
                    rightMask = col == WHITE ?
                        GenMask(boardPos+1 + offset, Board::BoardFields, offset) :
                        GenMask(startpos+1, boardPos+1, offset);

                const uint64_t mainMask = col == WHITE ?
                    GenMask(boardPos + offset, Board::BoardFields, offset) :
                    GenMask(startpos, boardPos, offset);

                rv[col][msb] = mainMask | leftMask | rightMask;
            }
        }

        return rv;
    }();
};

#endif //FILEMAP_H
