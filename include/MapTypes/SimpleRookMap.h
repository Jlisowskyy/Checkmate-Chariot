//
// Created by Jlisowskyy on 2/8/24.
//

#ifndef SIMPLEROOKMAP_H
#define SIMPLEROOKMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/RookMapGenerator.h"

class SimpleRookMap {
    using _hashFuncT = BaseHashFunction<true>;
    using _underlyingMapT = movesHashMap<_hashFuncT>;
public:
    constexpr SimpleRookMap() {
        for (int i = 0; i < Board::BoardFields; ++i) {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(RookMapGenerator::InitMasks(boardIndex), funcs[i]);
            MoveInitializer(_maps[i],
            [](const uint64_t n, const int ind) constexpr { return RookMapGenerator::GenMoves(n, ind); },
            [](const int ind, const RookMapGenerator::MasksT& m) constexpr { return RookMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
            [](const uint64_t b, const RookMapGenerator::MasksT& m) constexpr { return b; },
                boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const {
        const uint64_t closestNeighbors = RookMapGenerator::StripBlockingNeighbors(fullBoard, _maps[msbInd].masks);

        return  _maps[msbInd][closestNeighbors];
    }

    static void ParameterSearch() {
        _underlyingMapT::FindHashParameters(
            funcs,
            [](const int ind, const RookMapGenerator::MasksT& m) { return RookMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
            [](const int bInd) { return RookMapGenerator::InitMasks(bInd); }
        );
    }

    // ------------------------------
    // class fields
    // ------------------------------

private:

    static constexpr BaseHashFunction<true> funcs[Board::BoardFields] {
        _hashFuncT(std::make_tuple(3393641422875280979LLU, 17312422767356678212LLU, 67, 64)),
        _hashFuncT(std::make_tuple(5471342235767773913LLU, 17307815045900276771LLU, 67, 64)),
        _hashFuncT(std::make_tuple(15250091735978237630LLU, 16356769246725350830LLU, 131, 128)),
        _hashFuncT(std::make_tuple(12442485387656305543LLU, 16198993108558860794LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(9676328163961630310LLU, 13570260835205647813LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(216587437274599482LLU, 10929190736371504000LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(9048537097603897538LLU, 8504663955913029650LLU, 131, 128)),
        _hashFuncT(std::make_tuple(16622022989650590567LLU, 697641190516267145LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 257, 256)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 257, 256)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 257, 256)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 257, 256)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(9112182513768572659LLU, 17493466365116381458LLU, 131, 128)),
        _hashFuncT(std::make_tuple(9321354766185832861LLU, 10023829598634898570LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(14235290041803269772LLU, 17014087199960585519LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(13178403195126074399LLU, 3429497423754197273LLU, 67, 64)),
        _hashFuncT(std::make_tuple(2182512708681601225LLU, 17466145088073078408LLU, 67, 64)),
        _hashFuncT(std::make_tuple(17174721626244749098LLU, 2025289277575576374LLU, 67, 64)),
        _hashFuncT(std::make_tuple(6616896462523411835LLU, 7937903244653902784LLU, 67, 64)),
        _hashFuncT(std::make_tuple(17204342939815526595LLU, 14168889339532498898LLU, 131, 128)),
        _hashFuncT(std::make_tuple(9846749713679825003LLU, 4953919125260249464LLU, 131, 128)),
        _hashFuncT(std::make_tuple(18181425027871411485LLU, 8170902763400531562LLU, 131, 128)),
        _hashFuncT(std::make_tuple(17506460483279960373LLU, 7554106665307752666LLU, 131, 128)),
        _hashFuncT(std::make_tuple(15969999706032231071LLU, 8765763996995066529LLU, 67, 64)),
        _hashFuncT(std::make_tuple(4954199076915290639LLU, 7109284141322176083LLU, 67, 64)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //SIMPLEROOKMAP_H
