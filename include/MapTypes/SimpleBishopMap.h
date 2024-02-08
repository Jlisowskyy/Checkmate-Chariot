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
        HFuncT(std::make_tuple(7277981665153169073LLU, 16533625891010514269LLU, 11, 8)),
        HFuncT(std::make_tuple(4817405075091174023LLU, 12137389797463701647LLU, 11, 8)),
        HFuncT(std::make_tuple(10354607012174535240LLU, 10620236037258921160LLU, 17, 16)),
        HFuncT(std::make_tuple(7213451563617574610LLU, 16606136274651621034LLU, 17, 16)),
        HFuncT(std::make_tuple(8648953397424896771LLU, 9033580528762309994LLU, 17, 16)),
        HFuncT(std::make_tuple(13793900036815614736LLU, 5128434818345557158LLU, 17, 16)),
        HFuncT(std::make_tuple(13853280680093245353LLU, 12391792261901014109LLU, 11, 8)),
        HFuncT(std::make_tuple(11222913293301419086LLU, 15553432588746251840LLU, 11, 8)),
        HFuncT(std::make_tuple(6777231733473332589LLU, 13665519357572307065LLU, 11, 8)),
        HFuncT(std::make_tuple(1793682622039698594LLU, 17440454466910810315LLU, 11, 8)),
        HFuncT(std::make_tuple(15598373959583491714LLU, 15843111216247404648LLU, 17, 16)),
        HFuncT(std::make_tuple(11630010704284336274LLU, 9474720819999065208LLU, 17, 16)),
        HFuncT(std::make_tuple(15896497734355182817LLU, 8104285822391442589LLU, 17, 16)),
        HFuncT(std::make_tuple(2814921844542864585LLU, 8286010062219115339LLU, 17, 16)),
        HFuncT(std::make_tuple(15355553931485738685LLU, 5744092488486176748LLU, 11, 8)),
        HFuncT(std::make_tuple(5250268164591592732LLU, 4525629087847588365LLU, 11, 8)),
        HFuncT(std::make_tuple(13474766487216486312LLU, 2424065414397245882LLU, 17, 16)),
        HFuncT(std::make_tuple(2051812072996585467LLU, 13887950937545860267LLU, 17, 16)),
        HFuncT(std::make_tuple(565741000928214162LLU, 13371074675360998446LLU, 67, 64)),
        HFuncT(std::make_tuple(18303088406346790901LLU, 3441533038972102251LLU, 67, 64)),
        HFuncT(std::make_tuple(5289115046379996203LLU, 9989625116691135193LLU, 67, 64)),
        HFuncT(std::make_tuple(14715556204856476536LLU, 6322414150965391422LLU, 67, 64)),
        HFuncT(std::make_tuple(6185130227247975535LLU, 13717221831695625269LLU, 17, 16)),
        HFuncT(std::make_tuple(14558954320002173401LLU, 7675926062697317LLU, 17, 16)),
        HFuncT(std::make_tuple(14866272717292812780LLU, 4940571532417987949LLU, 17, 16)),
        HFuncT(std::make_tuple(7647125714713908393LLU, 11124283627329125313LLU, 17, 16)),
        HFuncT(std::make_tuple(13000301547095047488LLU, 10221080920016632012LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        HFuncT(std::make_tuple(2351640881230253287LLU, 1280636529999511744LLU, 67, 64)),
        HFuncT(std::make_tuple(4189797124149183524LLU, 18026114775350581627LLU, 17, 16)),
        HFuncT(std::make_tuple(2766224820243255398LLU, 5423835251553627664LLU, 17, 16)),
        HFuncT(std::make_tuple(3769345812131323739LLU, 9705521386366713355LLU, 17, 16)),
        HFuncT(std::make_tuple(4099614016501626763LLU, 8052959253975410250LLU, 17, 16)),
        HFuncT(std::make_tuple(13550773191248224582LLU, 4949665602032771405LLU, 67, 64)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        HFuncT(std::make_tuple(1LLU, 1LLU, 131, 128)),
        HFuncT(std::make_tuple(12294530689853026024LLU, 17235411587883121310LLU, 67, 64)),
        HFuncT(std::make_tuple(9615864549743167442LLU, 17223436379879937468LLU, 17, 16)),
        HFuncT(std::make_tuple(9618168486624750613LLU, 8445710091507928276LLU, 17, 16)),
        HFuncT(std::make_tuple(17565350887532589476LLU, 768856109385024301LLU, 17, 16)),
        HFuncT(std::make_tuple(9981134382404820562LLU, 10874649981364645087LLU, 17, 16)),
        HFuncT(std::make_tuple(3954736286852758344LLU, 436974091636137925LLU, 67, 64)),
        HFuncT(std::make_tuple(2702266512804153442LLU, 1041806833404766851LLU, 67, 64)),
        HFuncT(std::make_tuple(8052662336567212852LLU, 15283718408833780792LLU, 67, 64)),
        HFuncT(std::make_tuple(16222937449565037506LLU, 10401766394531321097LLU, 67, 64)),
        HFuncT(std::make_tuple(8501923477550511025LLU, 17860690512091926619LLU, 17, 16)),
        HFuncT(std::make_tuple(17725359354031829887LLU, 7508622379602700567LLU, 17, 16)),
        HFuncT(std::make_tuple(7865381024037554210LLU, 6460395907742683217LLU, 11, 8)),
        HFuncT(std::make_tuple(3042855833883176434LLU, 12977000670426590916LLU, 11, 8)),
        HFuncT(std::make_tuple(5018764814479062032LLU, 16860376138906378975LLU, 17, 16)),
        HFuncT(std::make_tuple(589088149035099792LLU, 16222164712469100257LLU, 17, 16)),
        HFuncT(std::make_tuple(7376312796240744193LLU, 15007217811426180703LLU, 17, 16)),
        HFuncT(std::make_tuple(3001903503554851175LLU, 11606636602190298050LLU, 17, 16)),
        HFuncT(std::make_tuple(13784453586291706367LLU, 5561177516030672686LLU, 11, 8)),
        HFuncT(std::make_tuple(8060414485848746774LLU, 2657178029372805182LLU, 11, 8)),
        HFuncT(std::make_tuple(8341970300478495570LLU, 16009993004451944250LLU, 11, 8)),
        HFuncT(std::make_tuple(10551218717963752706LLU, 7035976906297835190LLU, 11, 8)),
        HFuncT(std::make_tuple(558760440129325107LLU, 5190975052461767228LLU, 17, 16)),
        HFuncT(std::make_tuple(6047692745554873184LLU, 12143693349456769736LLU, 17, 16)),
        HFuncT(std::make_tuple(8971524967741809704LLU, 6051136899133467856LLU, 17, 16)),
        HFuncT(std::make_tuple(4784088459117050047LLU, 5645448585517400002LLU, 17, 16)),
        HFuncT(std::make_tuple(5218260134595256169LLU, 18017465147998176018LLU, 11, 8)),
        HFuncT(std::make_tuple(12847281603932373635LLU, 9635513767646835776LLU, 11, 8)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //SIMPLEBISHOPMAP_H
