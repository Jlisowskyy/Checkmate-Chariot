//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef ROOKMAP_H
#define ROOKMAP_H

#include <array>

#include "../movesHashMap.h"
#include "../EngineTypeDefs.h"

class RookMap {
    static constexpr size_t MaxRookPossibleNeighbors = 144;

public:
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    RookMap() = delete;
    static void FindHashParameters();

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetMoves(int msbInd, uint64_t fullBoard);

    // ------------------------------
    // private methods
    // ------------------------------
private:

    static constexpr void _initMoves(std::array<movesHashMap, Board::BoardFields>& maps);
    static constexpr void _initMaps(std::array<movesHashMap, Board::BoardFields>& maps);
    [[nodiscard]] constexpr static std::array<uint64_t, movesHashMap::MasksCount> _initMasks(int bInd);
    [[nodiscard]] constexpr static uint64_t _genMoves(uint64_t neighbors, int bInd);
    [[nodiscard]] constexpr static std::tuple<std::array<uint64_t, MaxRookPossibleNeighbors>, size_t> _genPossibleNeighbors(int bInd, const movesHashMap& record);
    [[nodiscard]] static constexpr size_t _neighborLayoutPossibleCountOnField(int x, int y);

    // ------------------------------
    // Class inner types
    // ------------------------------

    enum maskInd {
        lMask,
        rMask,
        uMask,
        dMask,
    };

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint64_t aHashValues[Board::BoardFields] = {
        3393641422875280979LLU,
        5471342235767773913LLU,
        15250091735978237630LLU,
        12442485387656305543LLU,
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
        17312422767356678212LLU,
        17307815045900276771LLU,
        16356769246725350830LLU,
        16198993108558860794LLU,
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

    static constexpr const char* names[] = { "lMask", "rMask", "uMask", "dMask" };

    static constexpr std::array<movesHashMap, Board::BoardFields> layer1 = []() constexpr
    {
        std::array<movesHashMap, Board::BoardFields> maps;

        _initMaps(maps);
        _initMoves(maps);

        return maps;
    }();

};

#endif //ROOKMAP_H
