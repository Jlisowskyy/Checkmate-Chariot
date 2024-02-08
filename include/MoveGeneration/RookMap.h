//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef ROOKMAP_H
#define ROOKMAP_H

#include "../MapTypes/SimpleRookMap.h"

class RookMap {
    // -------------------------------
    // Underlying map definition
    // -------------------------------

    using _underlyingMap = SimpleRookMap;

public:
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    RookMap() = delete;
    ~RookMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard){
        return _map.GetMoves(msbInd, fullBoard);
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    static constexpr const char* names[] = { "lMask", "rMask", "uMask", "dMask" };

    static constexpr _underlyingMap _map{};
};

#endif //ROOKMAP_H