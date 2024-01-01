//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef ROOKMAP_H
#define ROOKMAP_H

#include "RookMapGenerator.h"

class RookMap {
public:
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    RookMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard){
        const movesHashMap& rec = layer1[msbInd];

        const uint64_t uPart = ExtractLsbBit(fullBoard & rec.masks[RookMapGenerator::uMask]);
        const uint64_t dPart = ExtractMsbBit(fullBoard & rec.masks[RookMapGenerator::dMask]);
        const uint64_t lPart = ExtractMsbBit(fullBoard & rec.masks[RookMapGenerator::lMask]);
        const uint64_t rPart = ExtractLsbBit(fullBoard & rec.masks[RookMapGenerator::rMask]);
        const uint64_t closestNeighbors = uPart | dPart | lPart | rPart;
        const uint64_t moves = layer1[msbInd][closestNeighbors];

        return moves;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    static constexpr const char* names[] = { "lMask", "rMask", "uMask", "dMask" };

    static constexpr std::array<movesHashMap, Board::BoardFields> layer1 = RookMapGenerator::GetMap();
};

#endif //ROOKMAP_H
