//
// Created by Jlisowskyy on 2/10/24.
//

#ifndef FANCYMAGICBISHOPMAP_H
#define FANCYMAGICBISHOPMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/BishopMapGenerator.h"
#include "../MoveGeneration/SparseRandomGenerator.h"

class FancyMagicBishopMap
{
    using _hashFuncT = FancyMagicHashFunction<SparseRandomGenerator<>>;
    using _underlyingMapT = movesHashMap<_hashFuncT, BishopMapGenerator::MaxPossibleNeighborsWithOverlap>;

public:
    constexpr FancyMagicBishopMap()
    {
        for (int i = 0; i < static_cast<int>(Board::BoardFields); ++i)
        {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(BishopMapGenerator::InitMasks(boardIndex), funcs[i]);
            _maps[i].InitFullMask();

            MoveInitializer(_maps[i],
                            [](const uint64_t n, const int ind) constexpr
                            {
                                return BishopMapGenerator::GenMoves(n, ind);
                            },
                            []([[maybe_unused]]const int, const BishopMapGenerator::MasksT&m) constexpr
                            {
                                return BishopMapGenerator::GenPossibleNeighborsWithOverlap(m);
                            },
                            [](const uint64_t b, const BishopMapGenerator::MasksT&m) constexpr
                            {
                                return BishopMapGenerator::StripBlockingNeighbors(b, m);
                            },
                            boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const
    {
        const uint64_t neighbors = fullBoard & _maps[msbInd].fullMask;
        return _maps[msbInd][neighbors];
    }

    static void ParameterSearch()
    {
        auto nGen = []([[maybe_unused]]const int, const BishopMapGenerator::MasksT&m)
        {
            return BishopMapGenerator::GenPossibleNeighborsWithOverlap(m);
        };
        auto mInit = [](const int bInd) { return BishopMapGenerator::InitMasks(bInd); };

        _underlyingMapT::FindCollidingIndices<
            decltype(nGen),
            decltype(mInit),
            [](const uint64_t n, const std::array<uint64_t, 4>&m)
            {
                return BishopMapGenerator::StripBlockingNeighbors(n, m);
            }
        >(
            funcs,
            nGen,
            mInit
        );

        _underlyingMapT::FindHashParameters<
            decltype(nGen),
            decltype(mInit),
            [](const uint64_t n, const std::array<uint64_t, 4>&m)
            {
                return BishopMapGenerator::StripBlockingNeighbors(n, m);
            }
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
    static constexpr _hashFuncT funcs[Board::BoardFields]{
        _hashFuncT(std::make_tuple(2459020380749179396LLU, 6)),
        _hashFuncT(std::make_tuple(18228596997040662761LLU, 5)),
        _hashFuncT(std::make_tuple(577023771530035456LLU, 5)),
        _hashFuncT(std::make_tuple(140742052938256LLU, 5)),
        _hashFuncT(std::make_tuple(4241559519926323204LLU, 5)),
        _hashFuncT(std::make_tuple(40533590659958800LLU, 5)),
        _hashFuncT(std::make_tuple(13573396270746166127LLU, 5)),
        _hashFuncT(std::make_tuple(15433831379842884746LLU, 6)),
        _hashFuncT(std::make_tuple(5458137257786957413LLU, 5)),
        _hashFuncT(std::make_tuple(9011666189571585LLU, 5)),
        _hashFuncT(std::make_tuple(72128546918695297LLU, 5)),
        _hashFuncT(std::make_tuple(3378867985317888LLU, 5)),
        _hashFuncT(std::make_tuple(6397499356811370666LLU, 5)),
        _hashFuncT(std::make_tuple(565157583650897LLU, 5)),
        _hashFuncT(std::make_tuple(2452495878734481824LLU, 5)),
        _hashFuncT(std::make_tuple(2305992547226353793LLU, 5)),
        _hashFuncT(std::make_tuple(5896716211751964931LLU, 5)),
        _hashFuncT(std::make_tuple(4652306653314547856LLU, 5)),
        _hashFuncT(std::make_tuple(162130170776389698LLU, 7)),
        _hashFuncT(std::make_tuple(11613292506489506816LLU, 7)),
        _hashFuncT(std::make_tuple(12494788516674558978LLU, 7)),
        _hashFuncT(std::make_tuple(288804355648307712LLU, 7)),
        _hashFuncT(std::make_tuple(5001228016187988525LLU, 5)),
        _hashFuncT(std::make_tuple(167298237218849LLU, 5)),
        _hashFuncT(std::make_tuple(7397172614331155718LLU, 5)),
        _hashFuncT(std::make_tuple(15882937967404034260LLU, 5)),
        _hashFuncT(std::make_tuple(40541527747936518LLU, 7)),
        _hashFuncT(std::make_tuple(4069501445375656064LLU, 9)),
        _hashFuncT(std::make_tuple(10083843241836806529LLU, 9)),
        _hashFuncT(std::make_tuple(6266797948639643237LLU, 7)),
        _hashFuncT(std::make_tuple(586034268766736388LLU, 5)),
        _hashFuncT(std::make_tuple(2308394151070795840LLU, 5)),
        _hashFuncT(std::make_tuple(10054851337067495605LLU, 5)),
        _hashFuncT(std::make_tuple(90353743543543936LLU, 5)),
        _hashFuncT(std::make_tuple(1189092139733139972LLU, 7)),
        _hashFuncT(std::make_tuple(12483134911496110085LLU, 9)),
        _hashFuncT(std::make_tuple(1223299049642410168LLU, 9)),
        _hashFuncT(std::make_tuple(4888956467406968000LLU, 7)),
        _hashFuncT(std::make_tuple(145246620002288640LLU, 5)),
        _hashFuncT(std::make_tuple(12456504737662980LLU, 5)),
        _hashFuncT(std::make_tuple(576601500060224000LLU, 5)),
        _hashFuncT(std::make_tuple(4619649999728459497LLU, 5)),
        _hashFuncT(std::make_tuple(5975714137870106383LLU, 7)),
        _hashFuncT(std::make_tuple(2932970949829467478LLU, 7)),
        _hashFuncT(std::make_tuple(2902574806744450484LLU, 7)),
        _hashFuncT(std::make_tuple(1688859317807109378LLU, 7)),
        _hashFuncT(std::make_tuple(9368613263417540672LLU, 5)),
        _hashFuncT(std::make_tuple(18307229971900796838LLU, 5)),
        _hashFuncT(std::make_tuple(9223443788588130432LLU, 5)),
        _hashFuncT(std::make_tuple(9228491680955777025LLU, 5)),
        _hashFuncT(std::make_tuple(10905209255181491571LLU, 5)),
        _hashFuncT(std::make_tuple(4415763513441LLU, 5)),
        _hashFuncT(std::make_tuple(5042597873833943191LLU, 5)),
        _hashFuncT(std::make_tuple(144418812266088580LLU, 5)),
        _hashFuncT(std::make_tuple(576495945332868224LLU, 5)),
        _hashFuncT(std::make_tuple(2314885401732973320LLU, 5)),
        _hashFuncT(std::make_tuple(1738429605722719888LLU, 6)),
        _hashFuncT(std::make_tuple(2395073496242704814LLU, 5)),
        _hashFuncT(std::make_tuple(2306125723593080833LLU, 5)),
        _hashFuncT(std::make_tuple(117392932654286848LLU, 5)),
        _hashFuncT(std::make_tuple(6726376817723420599LLU, 5)),
        _hashFuncT(std::make_tuple(1731642857114050572LLU, 5)),
        _hashFuncT(std::make_tuple(2306977739592179777LLU, 5)),
        _hashFuncT(std::make_tuple(18381131039969901408LLU, 6)),
    };
    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //FANCYMAGICBISHOPMAP_H
