//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef BISHOPMAP_H
#define BISHOPMAP_H

#include "../MapTypes/SimpleBishopMap.h"

class BishopMap {
    // -------------------------------
    // Underlying map definition
    // -------------------------------

    using _underlyingMap = SimpleBishopMap;

    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------
public:

    BishopMap() = delete;
    ~BishopMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) {
        return _map.GetMoves(msbInd, fullBoard);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    inline static const char* names[] = { "nwMask", "neMask", "swMask", "seMask" };

    static constexpr _underlyingMap _map{};
};

#endif //BISHOPMAP_H
