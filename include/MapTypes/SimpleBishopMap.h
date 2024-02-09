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
    using _hashFuncT = BaseHashFunction<true>;
    using _underlyingMapT = movesHashMap<_hashFuncT>;
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
	_hashFuncT(std::make_tuple(2540615534426442431LLU, 18161687812890534152LLU, 11, 8)),
	_hashFuncT(std::make_tuple(11237402613493751323LLU, 8003405968917746827LLU, 11, 8)),
	_hashFuncT(std::make_tuple(9510716373887163911LLU, 3524171947865153647LLU, 17, 16)),
	_hashFuncT(std::make_tuple(6449400174864287381LLU, 7356523049413390067LLU, 17, 16)),
	_hashFuncT(std::make_tuple(12665527288850002160LLU, 3174498411009562157LLU, 17, 16)),
	_hashFuncT(std::make_tuple(4853634482874923740LLU, 10774850018386523877LLU, 17, 16)),
	_hashFuncT(std::make_tuple(18188589664130705605LLU, 15631639931568946807LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(8693478209756545513LLU, 4506678441337439676LLU, 11, 8)),
	_hashFuncT(std::make_tuple(4585269150775790892LLU, 7807210325974365171LLU, 11, 8)),
	_hashFuncT(std::make_tuple(7869510348008695377LLU, 7812956633162111118LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1443005652786245088LLU, 17157700586518416221LLU, 17, 16)),
	_hashFuncT(std::make_tuple(13031468250171439383LLU, 16910704318291963571LLU, 17, 16)),
	_hashFuncT(std::make_tuple(669930446377799544LLU, 4923447669805025047LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(10139111774217651801LLU, 14841578872426480958LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1476600364530001388LLU, 2707978354268175262LLU, 17, 16)),
	_hashFuncT(std::make_tuple(12022167654595185988LLU, 7833534493593215760LLU, 17, 16)),
	_hashFuncT(std::make_tuple(15648169580968091049LLU, 7969108857820326880LLU, 67, 64)),
	_hashFuncT(std::make_tuple(9609937080274780492LLU, 11889220498155803180LLU, 67, 64)),
	_hashFuncT(std::make_tuple(6350550981077548190LLU, 16608938017224185812LLU, 67, 64)),
	_hashFuncT(std::make_tuple(3538085614934284469LLU, 16552676330972419759LLU, 67, 64)),
	_hashFuncT(std::make_tuple(14267016668354064020LLU, 16524658945140172068LLU, 17, 16)),
	_hashFuncT(std::make_tuple(12815490474586626762LLU, 680652790632923090LLU, 17, 16)),
	_hashFuncT(std::make_tuple(3922129198458549334LLU, 7272362383016809233LLU, 17, 16)),
	_hashFuncT(std::make_tuple(16385448147481807970LLU, 4622851296883138035LLU, 17, 16)),
	_hashFuncT(std::make_tuple(3958742332006820590LLU, 8446035271446045565LLU, 67, 64)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 67, 64)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 17, 16)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
	_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
};

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //SIMPLEBISHOPMAP_H
