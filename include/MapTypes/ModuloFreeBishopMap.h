//
// Created by Jlisowskyy on 2/9/24.
//

#ifndef MODULOFREEBISHOPMAP_H
#define MODULOFREEBISHOPMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/BishopMapGenerator.h"

class ModuloFreeBishopMap {
    using _hashFuncT = Fast2PowHashFunction;
    using _underlyingMapT = movesHashMap<_hashFuncT>;
public:
    constexpr ModuloFreeBishopMap() {
        for (int i = 0; i < Board::BoardFields; ++i) {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(BishopMapGenerator::InitMasks(boardIndex), funcs[i]);
            MoveInitializer(_maps[i],
                [](const uint64_t n, const int ind) constexpr { return BishopMapGenerator::GenMoves(n, ind); },
                [](const int ind, const BishopMapGenerator::MasksT& m) constexpr { return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
                [](const uint64_t b, const BishopMapGenerator::MasksT& m) constexpr { return b; },
                boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const {
        const uint64_t closestNeighbors = BishopMapGenerator::StripBlockingNeighbors(fullBoard, _maps[msbInd].masks);

        return _maps[msbInd][closestNeighbors];
    }

    static void ParameterSearch() {
        _underlyingMapT::FindHashParameters(
            funcs,
            [](const int ind, const BishopMapGenerator::MasksT& m) { return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
            [](const int bInd) { return BishopMapGenerator::InitMasks(bInd); }
        );
    }

    // ------------------------------
    // class fields
    // ------------------------------

private:

    static constexpr BaseHashFunction<true> funcs[Board::BoardFields] {
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 16)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 8)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //MODULOFREEBISHOPMAP_H