//
// Created by Jlisowskyy on 2/8/24.
//

#ifndef SIMPLEROOKMAP_H
#define SIMPLEROOKMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/RookMapGenerator.h"
#include "../MoveGeneration/SparseRandomGenerator.h"

class SimpleRookMap
{
    using _hashFuncT = BaseHashFunction<true, SparseRandomGenerator<>>;
    using _underlyingMapT = movesHashMap<_hashFuncT>;

public:
    constexpr SimpleRookMap()
    {
        for (int i = 0; i < Board::BoardFields; ++i)
        {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(RookMapGenerator::InitMasks(boardIndex), funcs[i]);
            MoveInitializer(_maps[i],
                            [](const uint64_t n, const int ind) constexpr
                            {
                                return RookMapGenerator::GenMoves(n, ind);
                            },
                            [](const int ind, const RookMapGenerator::MasksT&m) constexpr
                            {
                                return RookMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
                            },
                            [](const uint64_t b, const RookMapGenerator::MasksT&m) constexpr { return b; },
                            boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const
    {
        const uint64_t closestNeighbors = RookMapGenerator::StripBlockingNeighbors(fullBoard, _maps[msbInd].masks);

        return _maps[msbInd][closestNeighbors];
    }

    static void ParameterSearch()
    {
        _underlyingMapT::FindCollidingIndices(
            funcs,
            [](const int ind, const RookMapGenerator::MasksT&m)
            {
                return RookMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
            },
            [](const int bInd) { return RookMapGenerator::InitMasks(bInd); }
        );
        _underlyingMapT::FindHashParameters(
            funcs,
            [](const int ind, const RookMapGenerator::MasksT&m)
            {
                return RookMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
            },
            [](const int bInd) { return RookMapGenerator::InitMasks(bInd); }
        );
    }

    // ------------------------------
    // class fields
    // ------------------------------

private:
    static constexpr _hashFuncT funcs[Board::BoardFields]{
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(733268122259603725LLU, 6620135297317806181LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(15836272013730978182LLU, 14376545890221556823LLU, 131, 128)),
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
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
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
        _hashFuncT(std::make_tuple(4755801292404686849LLU, 144141576354926596LLU, 131, 128)),
        _hashFuncT(std::make_tuple(153141226149790722LLU, 9223442406672695808LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(3906158583554283572LLU, 3245672537936812268LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1283846586431998096LLU, 4818654493778647730LLU, 67, 64)),
        _hashFuncT(std::make_tuple(7570886583378447213LLU, 389231856379761922LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(7981014257963872575LLU, 1746899913735578134LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //SIMPLEROOKMAP_H
