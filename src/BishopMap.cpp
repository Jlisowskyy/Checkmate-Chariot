//
// Created by Jlisowskyy on 12/28/23.
//

#include "../include/MoveGeneration/BishopMap.h"

void BishopMap::FindHashParameters() {
    movesHashMap::FindHashParameters(aHashValues, bHashValues, layer1.data(),
                                     [](const int bInd, const movesHashMap& record) { return _genPossibleNeighbors(bInd, record); }
    );
}

constexpr uint64_t BishopMap::GetMoves(const int msbInd, const uint64_t fullBoard) {
    const movesHashMap& rec = layer1[msbInd];

    const uint64_t NWPart = ExtractLsbBit(fullBoard & rec.masks[nwMask]);
    const uint64_t NEPart = ExtractLsbBit(fullBoard & rec.masks[neMask]);
    const uint64_t SWPart = ExtractMsbBit(fullBoard & rec.masks[swMask]);
    const uint64_t SEPart = ExtractMsbBit(fullBoard & rec.masks[seMask]);
    const uint64_t closestNeighbors = NWPart | NEPart | SWPart | SEPart;
    const uint64_t moves = layer1[msbInd][closestNeighbors];

    return moves;
}

constexpr void BishopMap::_initMaps(std::array<movesHashMap, Board::BoardFields>& maps) {
    MapInitializer(aHashValues, bHashValues, maps,
                   [](const int x, const int y) { return _neighborLayoutPossibleCountOnField(x, y); },
                   [](const int bInd) { return _initMasks(bInd); }
    );
}

constexpr std::tuple<std::array<uint64_t, BishopMap::MaxBishopPossibleNeighbors>, size_t> BishopMap::_genPossibleNeighbors(const int bInd, const movesHashMap& record) {
    std::array<uint64_t, MaxBishopPossibleNeighbors> ret{};
    size_t usedFields = 0;

    const int x = bInd % 8;
    const int y = bInd / 8;

    const int NWBorder = bInd + 7*std::min(x, 7-y);
    const int NEBorder = bInd + 9*std::min(7-x, 7-y) ;
    const int SWBorder = bInd - 9*std::min(x, y);
    const int SEBorder = bInd - 7*std::min(7-x, y);

    const uint64_t bPos = 1LLU << bInd;
    for (int nw = bInd; nw <= NWBorder; nw += 7) {
        const uint64_t nwPos = minMsbPossible << nw;
        if (nwPos != bPos && (record.masks[nwMask] & nwPos) == 0)
            continue;

        for (int ne = bInd; ne <= NEBorder; ne +=9) {
            const uint64_t nePos = minMsbPossible << ne;
            if (nePos != bPos && (record.masks[neMask] & nePos) == 0)
                continue;

            for (int sw = bInd; sw >= SWBorder; sw-=9) {
                const uint64_t swPos = minMsbPossible << sw;
                if (swPos != bPos && (record.masks[swMask] & swPos) == 0)
                    continue;

                for (int se = bInd; se >= SEBorder; se-=7) {
                    const uint64_t sePos = minMsbPossible << se;
                    if (sePos != bPos && (record.masks[seMask] & sePos) == 0)
                        continue;

                    const uint64_t neighbor = (nwPos | nePos | swPos | sePos) ^ bPos;
                    ret[usedFields++] = neighbor;
                }
            }
        }
    }

    return {ret, usedFields};
}

constexpr void BishopMap::_initMoves(std::array<movesHashMap, Board::BoardFields>& maps) {
    MoveInitializer(maps,
                    [](const uint64_t neighbors, const int bInd) constexpr
                    { return _genMoves(neighbors, bInd); },
                    [](const int bInd, const movesHashMap& record) constexpr
                    { return _genPossibleNeighbors(bInd, record); }
    );
}

constexpr uint64_t BishopMap::_genMoves(const uint64_t neighbors, const int bInd) {
    const int y = bInd / 8;
    const int x = bInd % 8;

    const int NWBorder = bInd + 7*std::min(x, 7-y);
    const int NEBorder = bInd + 9*std::min(7-x, 7-y) ;
    const int SWBorder = bInd - 9*std::min(x, y);
    const int SEBorder = bInd - 7*std::min(7-x, y);

    uint64_t moves = 0;

    // NW direction moves
    moves |= GenSlidingMoves(neighbors, bInd, 7,
                             [&](const int b) { return b <= NWBorder; }
    );

    // NE direction moves
    moves |= GenSlidingMoves(neighbors, bInd, 9,
                             [&](const int b) { return b <= NEBorder; }
    );

    // SW direction moves
    moves |= GenSlidingMoves(neighbors, bInd, -9,
                             [&](const int b) { return b >= SWBorder; }
    );

    // SW direction moves
    moves |= GenSlidingMoves(neighbors, bInd, -7,
                             [&](const int b) { return b >= SEBorder; }
    );

    return moves;
}

constexpr size_t BishopMap::_neighborLayoutPossibleCountOnField(const int x, const int y) {
    const size_t nwCount = std::max(1, std::min(x, 7-y));
    const size_t neCount = std::max(1, std::min(7-x, 7-y));
    const size_t swCount = std::max(1, std::min(x, y));
    const size_t seCount = std::max(1, std::min(7-x, y));

    return nwCount * neCount * swCount * seCount;
}

constexpr std::array<uint64_t, movesHashMap::MasksCount> BishopMap::_initMasks(const int bInd) {
    std::array<uint64_t, movesHashMap::MasksCount> ret{};
    const int x = bInd % 8;
    const int y = bInd / 8;

    const int NEBorder = bInd + 9*std::max(0, std::min(7-x, 7-y) - 1);
    const int NWBorder = bInd + 7*std::max(0, std::min(x, 7-y) - 1);
    const int SEBorder = bInd - 7*std::max(0, std::min(7-x, y) - 1);
    const int SWBorder = bInd - 9*std::max(0, std::min(x, y) - 1);

    // neMask generation
    ret[neMask] = GenMask(NEBorder, bInd, 9, std::less_equal{});

    // nwMask generation
    ret[nwMask] = GenMask(NWBorder, bInd, 7, std::less_equal{});

    // seMask generation
    ret[seMask] = GenMask(SEBorder, bInd, -7, std::greater_equal{});

    // swMask generation
    ret[swMask] = GenMask(SWBorder, bInd, -9, std::greater_equal{});

    return ret;
}
