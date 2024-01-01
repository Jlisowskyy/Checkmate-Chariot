//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef BISHOPMAP_H
#define BISHOPMAP_H

#include "BishopMapGenerator.h"

class BishopMap {
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------
public:

    BishopMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) {
        const movesHashMap& rec = layer1[msbInd];

        const uint64_t NWPart = ExtractLsbBit(fullBoard & rec.masks[BishopMapGenerator::nwMask]);
        const uint64_t NEPart = ExtractLsbBit(fullBoard & rec.masks[BishopMapGenerator::neMask]);
        const uint64_t SWPart = ExtractMsbBit(fullBoard & rec.masks[BishopMapGenerator::swMask]);
        const uint64_t SEPart = ExtractMsbBit(fullBoard & rec.masks[BishopMapGenerator::seMask]);
        const uint64_t closestNeighbors = NWPart | NEPart | SWPart | SEPart;
        const uint64_t moves = layer1[msbInd][closestNeighbors];

        return moves;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    inline static const char* names[] = { "nwMask", "neMask", "swMask", "seMask" };

    static constexpr std::array<movesHashMap, Board::BoardFields> layer1 {};
};

#endif //BISHOPMAP_H
