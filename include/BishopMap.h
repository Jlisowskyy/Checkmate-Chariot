//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef BISHOPMAP_H
#define BISHOPMAP_H

#include "movesHashMap.h"
#include "EngineTypeDefs.h"
#include "MoveGeneration.h"

class BishopMap {
    static constexpr size_t MaxBishopPossibleNeighbors = 108;

public:
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    BishopMap();

    void FindHashParameters();

    // ------------------------------
    // Class interaction
    // ------------------------------

    void Tester() const {
        auto [pos, size] = _genPossibleNeighbors(63, layer1[0]);

        for (size_t i = 0; i < size; ++i){
            std::cout << "\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";
            DisplayMask(pos[i]);
        }
    }

    [[nodiscard]] uint64_t GetMoves(int msbInd, uint64_t fullBoard, uint64_t allyBoard) const;

    // ------------------------------
    // Class private methods
    // ------------------------------

    static constexpr std::tuple<std::array<uint64_t, MaxBishopPossibleNeighbors>, size_t> _genPossibleNeighbors(int bInd, const movesHashMap& record);
    constexpr void _initMoves();
    [[nodiscard]] constexpr static uint64_t _genMoves(uint64_t neighbors, int bInd);
    [[nodiscard]] static constexpr size_t _neighborLayoutPossibleCountOnField(int x, int y);
    [[nodiscard]] static constexpr std::array<uint64_t, movesHashMap::MasksCount> _initMasks(int bInd);
    void _initMaps();

    // ------------------------------
    // Class inner types
    // ------------------------------
private:

    enum maskInd {
        nwMask,
        neMask,
        swMask,
        seMask,
    };

    // ------------------------------
    // Class fields
    // ------------------------------

    inline static const char* names[] = { "nwMask", "neMask", "swMask", "seMask" };

    static constexpr uint64_t aHashValues[Board::BoardFields] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static constexpr uint64_t bHashValues[Board::BoardFields] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    movesHashMap layer1[Board::BoardFields];
};



#endif //BISHOPMAP_H
