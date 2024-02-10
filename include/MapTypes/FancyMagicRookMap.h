//
// Created by Jlisowskyy on 2/10/24.
//

#ifndef FANCYMAGICROOKMAP_H
#define FANCYMAGICROOKMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/RookMapGenerator.h"

class FancyMagicRookMap {
    using _hashFuncT = FancyMagicHashFunction<>;
    using _underlyingMapT = movesHashMap<_hashFuncT, 512>;
public:
    constexpr FancyMagicRookMap() {
        for (int i = 0; i < Board::BoardFields; ++i) {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(RookMapGenerator::InitMasks(boardIndex), funcs[i]);
			_maps[i].InitFullMask();

            MoveInitializer(_maps[i],
                [](const uint64_t n, const int ind) constexpr { return RookMapGenerator::GenMoves(n, ind); },
                [](const int ind, const RookMapGenerator::MasksT& m) constexpr { return RookMapGenerator::GenPossibleNeighborsWithOverlap(m); },
                [](const uint64_t b, const RookMapGenerator::MasksT& m) constexpr { return RookMapGenerator::StripBlockingNeighbors(b, m); },
                boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const {
        const uint64_t neighbors = fullBoard & _maps[msbInd].fullMask;
        return _maps[msbInd][neighbors];
    }

    static void ParameterSearch() {
		auto nGen = [](const int _, const RookMapGenerator::MasksT& m) { return RookMapGenerator::GenPossibleNeighborsWithOverlap(m); };
    	auto mInit = [](const int bInd) { return RookMapGenerator::InitMasks(bInd); };

    	_underlyingMapT::FindCollidingIndices<
    		decltype(nGen),
    		decltype(mInit),
    		[](const uint64_t n, const std::array<uint64_t, 4>& m) { return RookMapGenerator::StripBlockingNeighbors(n, m); }
    	>(
			funcs,
			nGen,
			mInit
		);

    	_underlyingMapT::FindHashParameters<
			decltype(nGen),
			decltype(mInit),
			[](const uint64_t n, const std::array<uint64_t, 4>& m) { return RookMapGenerator::StripBlockingNeighbors(n, m); }
    	>(
            funcs,
            nGen,
            mInit
        );
    }

    // ------------------------------
    // class fields
    // ------------------------------

private:

    static constexpr _hashFuncT funcs[Board::BoardFields] {
        _hashFuncT(std::make_tuple(1304888103081811984LLU, 6)),
        _hashFuncT(std::make_tuple(791063336263415924LLU, 5)),
        _hashFuncT(std::make_tuple(7822410574452518405LLU, 5)),
        _hashFuncT(std::make_tuple(1LLU, 5)),
        _hashFuncT(std::make_tuple(1LLU, 5)),
        _hashFuncT(std::make_tuple(4266126214361274349LLU, 5)),
        _hashFuncT(std::make_tuple(16636163426739556130LLU, 5)),
        _hashFuncT(std::make_tuple(11072786345544990642LLU, 6)),
        _hashFuncT(std::make_tuple(18098687923309769846LLU, 5)),
        _hashFuncT(std::make_tuple(7195744987761345694LLU, 5)),
        _hashFuncT(std::make_tuple(3986727869601871670LLU, 5)),
        _hashFuncT(std::make_tuple(15430707875468556340LLU, 5)),
        _hashFuncT(std::make_tuple(12022336319398551933LLU, 5)),
        _hashFuncT(std::make_tuple(5974210490437478907LLU, 5)),
        _hashFuncT(std::make_tuple(10709989180463875798LLU, 5)),
        _hashFuncT(std::make_tuple(3848749042695667254LLU, 5)),
        _hashFuncT(std::make_tuple(5896716211751964931LLU, 5)),
        _hashFuncT(std::make_tuple(2746782635093450504LLU, 5)),
        _hashFuncT(std::make_tuple(15702874870926707967LLU, 7)),
        _hashFuncT(std::make_tuple(11613292506489506816LLU, 7)),
        _hashFuncT(std::make_tuple(12494788516674558978LLU, 7)),
        _hashFuncT(std::make_tuple(3153820524763338242LLU, 7)),
        _hashFuncT(std::make_tuple(16284406411236988945LLU, 5)),
        _hashFuncT(std::make_tuple(1099037791890371407LLU, 5)),
        _hashFuncT(std::make_tuple(7397172614331155718LLU, 5)),
        _hashFuncT(std::make_tuple(15882937967404034260LLU, 5)),
        _hashFuncT(std::make_tuple(10402694421956853558LLU, 7)),
        _hashFuncT(std::make_tuple(4069501445375656064LLU, 9)),
        _hashFuncT(std::make_tuple(10083843241836806529LLU, 9)),
        _hashFuncT(std::make_tuple(6266797948639643237LLU, 7)),
        _hashFuncT(std::make_tuple(16797423475124582351LLU, 5)),
        _hashFuncT(std::make_tuple(5261299203380555532LLU, 5)),
        _hashFuncT(std::make_tuple(10054851337067495605LLU, 5)),
        _hashFuncT(std::make_tuple(18231622418956876490LLU, 5)),
        _hashFuncT(std::make_tuple(1820992464513543679LLU, 7)),
        _hashFuncT(std::make_tuple(12483134911496110085LLU, 9)), // 		_hashFuncT(std::make_tuple(512420793708126201LLU, 9)),
        _hashFuncT(std::make_tuple(1223299049642410168LLU, 9)),
        _hashFuncT(std::make_tuple(4888956467406968000LLU, 7)),
        _hashFuncT(std::make_tuple(5015872955111408367LLU, 5)),
        _hashFuncT(std::make_tuple(7034782627490706770LLU, 5)),
        _hashFuncT(std::make_tuple(17400956056348401736LLU, 5)),
        _hashFuncT(std::make_tuple(4619649999728459497LLU, 5)),
        _hashFuncT(std::make_tuple(5975714137870106383LLU, 7)),
        _hashFuncT(std::make_tuple(2932970949829467478LLU, 7)),
        _hashFuncT(std::make_tuple(2902574806744450484LLU, 7)),
        _hashFuncT(std::make_tuple(1688859317807109378LLU, 7)),
        _hashFuncT(std::make_tuple(6061795778347790648LLU, 5)),
        _hashFuncT(std::make_tuple(18307229971900796838LLU, 5)),
        _hashFuncT(std::make_tuple(17965068854548627547LLU, 5)),
        _hashFuncT(std::make_tuple(16628456126524129413LLU, 5)),
        _hashFuncT(std::make_tuple(10905209255181491571LLU, 5)),
        _hashFuncT(std::make_tuple(15279504055131985538LLU, 5)),
        _hashFuncT(std::make_tuple(5042597873833943191LLU, 5)),
        _hashFuncT(std::make_tuple(10685254548740368231LLU, 5)),
        _hashFuncT(std::make_tuple(5873211263693006843LLU, 5)),
        _hashFuncT(std::make_tuple(6554152907125806591LLU, 5)),
        _hashFuncT(std::make_tuple(17911104566945579206LLU, 6)),
        _hashFuncT(std::make_tuple(2395073496242704814LLU, 5)),
        _hashFuncT(std::make_tuple(12909519480940001285LLU, 5)),
        _hashFuncT(std::make_tuple(16968431856099893139LLU, 5)),
        _hashFuncT(std::make_tuple(6726376817723420599LLU, 5)),
        _hashFuncT(std::make_tuple(13044608236556692757LLU, 5)),
        _hashFuncT(std::make_tuple(4783150144548767711LLU, 5)),
        _hashFuncT(std::make_tuple(18381131039969901408LLU, 6)),
};

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //FANCYMAGICROOKMAP_H
