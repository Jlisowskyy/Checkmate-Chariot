//
// Created by Jlisowskyy on 2/12/24.
//

#ifndef PINNINGMASK_H
#define PINNINGMASK_H

#include <array>
#include <cinttypes>
#include "RookMapGenerator.h"
#include "BishopMapGenerator.h"

struct PinningMasks
{
    // ------------------------------
    // Class creation
    // ------------------------------

    PinningMasks() = default;

    PinningMasks(const PinningMasks&) = default;

    ~PinningMasks() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    explicit constexpr PinningMasks(const int bInd)
    {
        constexpr int northBarrier = 64;
        constexpr int southBarrier = -1;

        const int y = bInd / 8;
        const int x = bInd % 8;

        const int westBarrier = ((bInd / 8) * 8) - 1;
        const int eastBarrier = westBarrier + 9;
        const int nwBorder = bInd + 7 * std::min(x, 7 - y);
        const int neBorder = bInd + 9 * std::min(7 - x, 7 - y);
        const int swBorder = bInd - 9 * std::min(x, y);
        const int seBorder = bInd - 7 * std::min(7 - x, y);

        // Rook-like lines
        masks[NorthMask] = GenMask(northBarrier, bInd, RookMapGenerator::NorthOffset, std::less{});
        masks[SouthMask] = GenMask(southBarrier, bInd, RookMapGenerator::SouthOffset, std::greater{});
        masks[EastMask] = GenMask(eastBarrier, bInd, RookMapGenerator::EastOffset, std::less{});
        masks[WestMask] = GenMask(westBarrier, bInd, RookMapGenerator::WestOffset, std::greater{});

        // Bishop-like lines
        masks[NorthEastMask] = GenMask(neBorder, bInd, BishopMapGenerator::NEOffset, std::less_equal{});
        masks[NorthWestMask] = GenMask(nwBorder, bInd, BishopMapGenerator::NWOffset, std::less_equal{});
        masks[SouthEastMask] = GenMask(seBorder, bInd, BishopMapGenerator::SEOffset, std::greater_equal{});
        masks[SouthWestMask] = GenMask(swBorder, bInd, BishopMapGenerator::SWOffset, std::greater_equal{});

        for (int i = 0; i < PinningMasksCount / 2; ++i) rookMask |= masks[i];
        for (int i = PinningMasksCount / 2; i < PinningMasksCount; ++i) bishopMask |= masks[i];
        rookMask |= (1LLU << bInd);
        bishopMask |= (1LLU << bInd);
        fullMask = bishopMask | rookMask;
    }

    static constexpr std::array<PinningMasks, Board::BoardFields> PinningArrayFactory()
    {
        std::array<PinningMasks, Board::BoardFields> maskArr{};
        for (int i = 0; i < Board::BoardFields; ++i) maskArr[i] = PinningMasks(ConvertToReversedPos(i));
        return maskArr;
    }

    // ------------------------------
    // Class Fields
    // ------------------------------

    static constexpr size_t PinningMasksCount = 8;
    static constexpr size_t PinningMaskPerLinesType = 4;
    static constexpr size_t BishopLines = 0;
    static constexpr size_t RookLines = 4;
    std::array<uint64_t, PinningMasksCount> masks{};
    uint64_t fullMask{};
    uint64_t rookMask{};;
    uint64_t bishopMask{};

    // ------------------------------
    // Class inner types
    // ------------------------------

    enum PinnigMaskssNames
    {
        NorthMask,
        SouthMask,
        WestMask,
        EastMask,
        NorthWestMask,
        SouthEastMask,
        NorthEastMask,
        SouthWestMask,
    };
};

#endif //PINNINGMASK_H
