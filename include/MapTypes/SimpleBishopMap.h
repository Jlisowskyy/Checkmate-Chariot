//
// Created by Jlisowskyy on 2/5/24.
//

#ifndef SIMPLEBISHOPMAP_H
#define SIMPLEBISHOPMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/BishopMapGenerator.h"

class SimpleBishopMap {
    using HFuncT = BaseHashFunction<true>;
    using _underlyingMapT = movesHashMap<HFuncT>;
public:
    constexpr SimpleBishopMap() {
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
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //SIMPLEBISHOPMAP_H
