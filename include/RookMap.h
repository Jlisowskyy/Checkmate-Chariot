//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef ROOKMAP_H
#define ROOKMAP_H

#include <array>

#include "BitOperations.h"
#include "movesHashMap.h"
#include "EngineTypeDefs.h"

struct RookMap {
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    RookMap();
    void FindHashParameters();
    [[nodiscard]] uint64_t GetMoves(int msbInd, uint64_t fullBoard, uint64_t allyBoard) const;

    // ------------------------------
    // private methods
    // ------------------------------
private:

    template<class incFunc, class boundryCheckFunc>
    [[nodiscard]] constexpr static uint64_t _genMovesOnLine(uint64_t neighbors, int bInd,
        incFunc inc, boundryCheckFunc boundryCheck);

    template<class incFunc, class boundryCheckFunc>
    constexpr static uint64_t _genMask(int barrier, int boardIndex, incFunc inc, boundryCheckFunc boundryCheck);

    void _integrityTest();
    constexpr void _initMoves();
    void _initMaps();
    [[nodiscard]] static std::array<uint64_t, movesHashMap::MasksCount> _initMasks(int bInd);
    [[nodiscard]] constexpr static uint64_t _genMoves(uint64_t neighbors, int bInd);
    constexpr static uint64_t _genRMask(int barrier, int boardIndex);
    constexpr static uint64_t _genLMask(int barrier, int boardIndex);
    constexpr static uint64_t _genUMask(int boardIndex);
    constexpr static uint64_t _genDMask(int boardIndex);
    static constexpr size_t MaxRookPossibleNeighbors = 144;
    constexpr static std::tuple<std::array<uint64_t, MaxRookPossibleNeighbors>, size_t> _genPossibleNeighbors(int bInd, const movesHashMap& record);
    static constexpr uint64_t _genModuloMask(size_t modSize);
    static constexpr size_t _neighborLayoutPossibleCountOnField(int x, int y);
    static constexpr size_t _calculatePossibleMovesCount();
    static void _displayMasks(const movesHashMap& map);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint64_t aHashValues[Board::BoardFields] = {
        0,
        0,
        0,
        0,
        0,
        0,
        9676328163961630310LLU,
        0,
        0,
        0,
        0,
        0,
        216587437274599482LLU,
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
        9048537097603897538LLU,
        16622022989650590567LLU,
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
        9112182513768572659LLU,
        9321354766185832861LLU,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        14235290041803269772LLU,
        0,
        0,
        0,
        0,
        13178403195126074399LLU,
        2182512708681601225LLU,
        17174721626244749098LLU,
        6616896462523411835LLU,
        17204342939815526595LLU,
        9846749713679825003LLU,
        18181425027871411485LLU,
        17506460483279960373LLU,
        15969999706032231071LLU,
        4954199076915290639LLU,
    };

    static constexpr uint64_t bHashValues[Board::BoardFields] = {
        0,
        0,
        0,
        0,
        0,
        0,
        13570260835205647813LLU,
        0,
        0,
        0,
        0,
        0,
        10929190736371504000LLU,
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
        8504663955913029650LLU,
        697641190516267145LLU,
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
        17493466365116381458LLU,
        10023829598634898570LLU,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        17014087199960585519LLU,
        0,
        0,
        0,
        0,
        3429497423754197273LLU,
        17466145088073078408LLU,
        2025289277575576374LLU,
        7937903244653902784LLU,
        14168889339532498898LLU,
        4953919125260249464LLU,
        8170902763400531562LLU,
        7554106665307752666LLU,
        8765763996995066529LLU,
        7109284141322176083LLU,
    };

    enum maskInd {
        lMask,
        rMask,
        uMask,
        dMask,
    };

    movesHashMap layer1[Board::BoardFields];
};

template<class incFunc, class boundryCheckFunc>
constexpr uint64_t RookMap::_genMovesOnLine(const uint64_t neighbors, const int bInd, incFunc inc,
    boundryCheckFunc boundryCheck) {
    uint64_t ret = 0;
    int actPos = bInd;

    while (boundryCheck(actPos = inc(actPos))) {
        const uint64_t curMove = 1LLU << actPos;
        ret |= curMove;

        if ((curMove & neighbors) != 0) break;
    }

    return ret;
}

template<class incFunc, class boundryCheckFunc>
constexpr uint64_t RookMap::_genMask(int barrier, int boardIndex, incFunc inc, boundryCheckFunc boundryCheck) {
    uint64_t mask = 0;

    while(boundryCheck(boardIndex = inc(boardIndex), barrier))
        mask |= (1LLU<<boardIndex);

    return mask;
}

#endif //ROOKMAP_H
