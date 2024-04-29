//
// Created by Jlisowskyy on 3/26/24.
//

#ifndef FILEMAP_H
#define FILEMAP_H

#include "MoveGenerationUtils.h"

struct FileMap
{
    using SepFiles = const uint64_t *;

    // ------------------------------
    // Class creation
    // ------------------------------

    FileMap()  = delete;
    ~FileMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    static uint64_t GetPlainFile(const int msbPos) { return _plainFiles[msbPos]; }

    static uint64_t GetNeighborFiles(const int msbPos) { return _neigborFiles[msbPos]; }

    static uint64_t GetFatFile(const int msbPos) { return _fatFiles[msbPos]; }

    static uint64_t GetFronFatFile(const int msbPos, const int col) { return _frontFatFiles[col][msbPos]; }

    static uint64_t GetFrontFile(const int msbPos, const int col) { return _frontFiles[col][msbPos]; }

    static SepFiles GetSepFiles(const int msbPos) { return _plainSepFiles[msbPos].data(); }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    static constexpr std::array<uint64_t, Board::BitBoardFields> _plainFiles = []() constexpr
    {
        std::array<uint64_t, Board::BitBoardFields> rv{};

        for (int msb = 0; msb < static_cast<int>(Board::BitBoardFields); ++msb)
        {
            const int boardPos = ConvertToReversedPos(msb);
            const int startpos = boardPos % 8;
            rv[msb]            = GenMask(startpos, Board::BitBoardFields, 8);
        }

        return rv;
    }();

    static constexpr std::array<uint64_t, Board::BitBoardFields> _neigborFiles = []() constexpr
    {
        std::array<uint64_t, Board::BitBoardFields> rv{};

        for (int msb = 0; msb < static_cast<int>(Board::BitBoardFields); ++msb)
        {
            const int boardPos = ConvertToReversedPos(msb);
            const int startpos = boardPos % 8;

            uint64_t leftMap{};
            uint64_t rightMap{};

            if (startpos - 1 >= 0)
                leftMap = GenMask(startpos - 1, Board::BitBoardFields, 8);

            if (startpos + 1 < 8)
                rightMap = GenMask(startpos + 1, Board::BitBoardFields, 8);

            rv[msb] = leftMap | rightMap;
        }

        return rv;
    }();

    static constexpr std::array<uint64_t, Board::BitBoardFields> _fatFiles = []() constexpr
    {
        std::array<uint64_t, Board::BitBoardFields> rv{};

        for (int msb = 0; msb < static_cast<int>(Board::BitBoardFields); ++msb)
        {
            const int boardPos       = ConvertToReversedPos(msb);
            const int startpos       = boardPos % 8;
            const uint64_t leftMask  = startpos > 0 ? GenMask(startpos - 1, Board::BitBoardFields, 8) : 0;
            const uint64_t rightMask = startpos < 7 ? GenMask(startpos + 1, Board::BitBoardFields, 8) : 0;

            rv[msb] = GenMask(startpos, Board::BitBoardFields, 8) | leftMask | rightMask;
        }

        return rv;
    }();

    static constexpr int offset                                                                = 8;
    static constexpr std::array<std::array<uint64_t, Board::BitBoardFields>, 2> _frontFatFiles = []() constexpr
    {
        std::array<std::array<uint64_t, Board::BitBoardFields>, 2> rv{};

        for (int col = 0; col < 2; ++col)
        {
            for (int msb = 0; msb < static_cast<int>(Board::BitBoardFields); ++msb)
            {
                const int boardPos  = ConvertToReversedPos(msb);
                const int nFiledOff = col == WHITE ? offset : -offset;
                if (const int nextRow = boardPos + nFiledOff;
                    nextRow < 0 || nextRow >= static_cast<int>(Board::BitBoardFields))
                    continue;

                const int startpos = boardPos % offset;

                uint64_t leftMask{};
                uint64_t rightMask{};
                if (startpos > 0)
                    leftMask = col == WHITE ? GenMask(boardPos - 1 + offset, Board::BitBoardFields, offset)
                                            : GenMask(startpos - 1, boardPos - 1, offset);

                if (startpos < 7)
                    rightMask = col == WHITE ? GenMask(boardPos + 1 + offset, Board::BitBoardFields, offset)
                                             : GenMask(startpos + 1, boardPos + 1, offset);

                const uint64_t mainMask = col == WHITE ? GenMask(boardPos + offset, Board::BitBoardFields, offset)
                                                       : GenMask(startpos, boardPos, offset);

                rv[col][msb] = mainMask | leftMask | rightMask;
            }
        }

        return rv;
    }();

    static constexpr std::array<std::array<uint64_t, Board::BitBoardFields>, 2> _frontFiles = []() constexpr
    {
        std::array<std::array<uint64_t, Board::BitBoardFields>, 2> rv{};

        for (int col = 0; col < 2; ++col)
        {
            for (int msb = 0; msb < static_cast<int>(Board::BitBoardFields); ++msb)
            {
                const int boardPos  = ConvertToReversedPos(msb);
                const int nFiledOff = col == WHITE ? offset : -offset;
                if (const int nextRow = boardPos + nFiledOff;
                    nextRow < 0 || nextRow >= static_cast<int>(Board::BitBoardFields))
                    continue;

                const int startpos = boardPos % offset;

                rv[col][msb] = col == WHITE ? GenMask(boardPos + offset, Board::BitBoardFields, offset)
                                            : GenMask(startpos, boardPos, offset);
            }
        }

        return rv;
    }();

    public:
    static constexpr size_t FileSepSize = 3;

    private:
    static constexpr std::array<std::array<uint64_t, FileSepSize>, Board::BitBoardFields> _plainSepFiles =
        []() constexpr
    {
        std::array<std::array<uint64_t, FileSepSize>, Board::BitBoardFields> rv{};

        for (int msb = 0; msb < static_cast<int>(Board::BitBoardFields); ++msb)
        {
            const int boardPos = ConvertToReversedPos(msb);
            const int startpos = boardPos % 8;
            rv[msb]            = {
                startpos > 0 ? GenMask(startpos - 1, Board::BitBoardFields, 8) : 0,
                GenMask(startpos, Board::BitBoardFields, 8),
                startpos < 7 ? GenMask(startpos + 1, Board::BitBoardFields, 8) : 0
            };
        }

        return rv;
    }();
};

#endif // FILEMAP_H
