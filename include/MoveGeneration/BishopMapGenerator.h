//
// Created by Jlisowskyy on 1/1/24.
//

#ifndef BISHOPMAPGENERATOR_H
#define BISHOPMAPGENERATOR_H

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "MoveGeneration.h"

class BishopMapGenerator {
    static constexpr size_t MaxBishopPossibleNeighbors = 108;

    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------
public:

    BishopMapGenerator() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr std::array<movesHashMap, Board::BoardFields> GetMap() {
        std::array<movesHashMap, Board::BoardFields> maps{};

        _initMaps(maps);
        _initMoves(maps);

        return maps;
    }


    static void FindHashParameters(){
        movesHashMap::FindHashParameters(aHashValues, bHashValues, GetMap().data(),
                                         [](const int bInd, const movesHashMap& record) { return _genPossibleNeighbors(bInd, record); }
        );
    }

    // ------------------------------
    // Class private methods
    // ------------------------------
private:

    [[nodiscard]] static constexpr std::tuple<std::array<uint64_t, MaxBishopPossibleNeighbors>, size_t>
            _genPossibleNeighbors(const int bInd, const movesHashMap& record)
    {
        std::array<uint64_t, MaxBishopPossibleNeighbors> ret{};
        size_t usedFields = 0;

        const int x = bInd % 8;
        const int y = bInd / 8;

        const int NWBorder = bInd + 7*std::min(x, 7-y);
        const int NEBorder = bInd + 9*std::min(7-x, 7-y) ;
        const int SWBorder = bInd - 9*std::min(x, y);
        const int SEBorder = bInd - 7*std::min(7-x, y);

        const uint64_t bPos = 1LLU << bInd;
        for (int nw = bInd; nw <= NWBorder; nw += 7) {
            const uint64_t nwPos = minMsbPossible << nw;
            if (nwPos != bPos && (record.masks[nwMask] & nwPos) == 0)
                continue;

            for (int ne = bInd; ne <= NEBorder; ne +=9) {
                const uint64_t nePos = minMsbPossible << ne;
                if (nePos != bPos && (record.masks[neMask] & nePos) == 0)
                    continue;

                for (int sw = bInd; sw >= SWBorder; sw-=9) {
                    const uint64_t swPos = minMsbPossible << sw;
                    if (swPos != bPos && (record.masks[swMask] & swPos) == 0)
                        continue;

                    for (int se = bInd; se >= SEBorder; se-=7) {
                        const uint64_t sePos = minMsbPossible << se;
                        if (sePos != bPos && (record.masks[seMask] & sePos) == 0)
                            continue;

                        const uint64_t neighbor = (nwPos | nePos | swPos | sePos) ^ bPos;
                        ret[usedFields++] = neighbor;
                    }
                }
            }
        }

        return {ret, usedFields};
    }

    static constexpr void _initMoves(std::array<movesHashMap, Board::BoardFields>& maps){
        MoveInitializer(maps,
                        [](const uint64_t neighbors, const int bInd) constexpr
                        { return _genMoves(neighbors, bInd); },
                        [](const int bInd, const movesHashMap& record) constexpr
                        { return _genPossibleNeighbors(bInd, record); }
        );
    }

    [[nodiscard]] static constexpr uint64_t _genMoves(const uint64_t neighbors, const int bInd){
        const int y = bInd / 8;
        const int x = bInd % 8;

        const int NWBorder = bInd + 7*std::min(x, 7-y);
        const int NEBorder = bInd + 9*std::min(7-x, 7-y) ;
        const int SWBorder = bInd - 9*std::min(x, y);
        const int SEBorder = bInd - 7*std::min(7-x, y);

        uint64_t moves = 0;

        // NW direction moves
        moves |= GenSlidingMoves(neighbors, bInd, 7,
                                 [&](const int b) { return b <= NWBorder; }
        );

        // NE direction moves
        moves |= GenSlidingMoves(neighbors, bInd, 9,
                                 [&](const int b) { return b <= NEBorder; }
        );

        // SW direction moves
        moves |= GenSlidingMoves(neighbors, bInd, -9,
                                 [&](const int b) { return b >= SWBorder; }
        );

        // SW direction moves
        moves |= GenSlidingMoves(neighbors, bInd, -7,
                                 [&](const int b) { return b >= SEBorder; }
        );

        return moves;
    }

    [[nodiscard]] static constexpr size_t _neighborLayoutPossibleCountOnField(int x, int y){
        const size_t nwCount = std::max(1, std::min(x, 7-y));
        const size_t neCount = std::max(1, std::min(7-x, 7-y));
        const size_t swCount = std::max(1, std::min(x, y));
        const size_t seCount = std::max(1, std::min(7-x, y));

        return nwCount * neCount * swCount * seCount;
    }


    [[nodiscard]] static constexpr std::array<uint64_t, movesHashMap::MasksCount> _initMasks(const int bInd){
        std::array<uint64_t, movesHashMap::MasksCount> ret{};
        const int x = bInd % 8;
        const int y = bInd / 8;

        const int NEBorder = bInd + 9*std::max(0, std::min(7-x, 7-y) - 1);
        const int NWBorder = bInd + 7*std::max(0, std::min(x, 7-y) - 1);
        const int SEBorder = bInd - 7*std::max(0, std::min(7-x, y) - 1);
        const int SWBorder = bInd - 9*std::max(0, std::min(x, y) - 1);

        // neMask generation
        ret[neMask] = GenMask(NEBorder, bInd, 9, std::less_equal{});

        // nwMask generation
        ret[nwMask] = GenMask(NWBorder, bInd, 7, std::less_equal{});

        // seMask generation
        ret[seMask] = GenMask(SEBorder, bInd, -7, std::greater_equal{});

        // swMask generation
        ret[swMask] = GenMask(SWBorder, bInd, -9, std::greater_equal{});

        return ret;
    }

    static constexpr void _initMaps(std::array<movesHashMap, Board::BoardFields>& maps){
        MapInitializer(aHashValues, bHashValues, maps,
                       [](const int x, const int y) { return _neighborLayoutPossibleCountOnField(x, y); },
                       [](const int bInd) { return _initMasks(bInd); }
        );
    }


    // ------------------------------
    // Class inner types
    // ------------------------------
public:

    enum maskInd {
        nwMask,
        neMask,
        swMask,
        seMask,
    };

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    static constexpr uint64_t aHashValues[Board::BoardFields] = {
        7277981665153169073LLU,
        4817405075091174023LLU,
        10354607012174535240LLU,
        7213451563617574610LLU,
        8648953397424896771LLU,
        13793900036815614736LLU,
        13853280680093245353LLU,
        11222913293301419086LLU,
        6777231733473332589LLU,
        1793682622039698594LLU,
        15598373959583491714LLU,
        11630010704284336274LLU,
        15896497734355182817LLU,
        2814921844542864585LLU,
        15355553931485738685LLU,
        5250268164591592732LLU,
        13474766487216486312LLU,
        2051812072996585467LLU,
        565741000928214162LLU,
        18303088406346790901LLU,
        5289115046379996203LLU,
        14715556204856476536LLU,
        6185130227247975535LLU,
        14558954320002173401LLU,
        14866272717292812780LLU,
        7647125714713908393LLU,
        13000301547095047488LLU,
        0LLU,
        0LLU,
        2351640881230253287LLU,
        4189797124149183524LLU,
        2766224820243255398LLU,
        3769345812131323739LLU,
        4099614016501626763LLU,
        13550773191248224582LLU,
        0LLU,
        0LLU,
        0LLU,
        0LLU,
        9618168486624750613LLU,
        17565350887532589476LLU,
        9981134382404820562LLU,
        3954736286852758344LLU,
        2702266512804153442LLU,
        8052662336567212852LLU,
        16222937449565037506LLU,
        8501923477550511025LLU,
        17725359354031829887LLU,
        7865381024037554210LLU,
        3042855833883176434LLU,
        5018764814479062032LLU,
        589088149035099792LLU,
        7376312796240744193LLU,
        3001903503554851175LLU,
        13784453586291706367LLU,
        8060414485848746774LLU,
        8341970300478495570LLU,
        10551218717963752706LLU,
        558760440129325107LLU,
        6047692745554873184LLU,
        8971524967741809704LLU,
        4784088459117050047LLU,
        5218260134595256169LLU,
        12847281603932373635LLU,
    };

    static constexpr uint64_t bHashValues[Board::BoardFields] = {
        16533625891010514269LLU,
        12137389797463701647LLU,
        10620236037258921160LLU,
        16606136274651621034LLU,
        9033580528762309994LLU,
        5128434818345557158LLU,
        12391792261901014109LLU,
        15553432588746251840LLU,
        13665519357572307065LLU,
        17440454466910810315LLU,
        15843111216247404648LLU,
        9474720819999065208LLU,
        8104285822391442589LLU,
        8286010062219115339LLU,
        5744092488486176748LLU,
        4525629087847588365LLU,
        2424065414397245882LLU,
        13887950937545860267LLU,
        13371074675360998446LLU,
        3441533038972102251LLU,
        9989625116691135193LLU,
        6322414150965391422LLU,
        13717221831695625269LLU,
        7675926062697317LLU,
        4940571532417987949LLU,
        11124283627329125313LLU,
        10221080920016632012LLU,
        0LLU,
        0LLU,
        1280636529999511744LLU,
        18026114775350581627LLU,
        5423835251553627664LLU,
        9705521386366713355LLU,
        8052959253975410250LLU,
        4949665602032771405LLU,
        0LLU,
        0LLU,
        0LLU,
        0LLU,
        8445710091507928276LLU,
        768856109385024301LLU,
        10874649981364645087LLU,
        436974091636137925LLU,
        1041806833404766851LLU,
        15283718408833780792LLU,
        10401766394531321097LLU,
        17860690512091926619LLU,
        7508622379602700567LLU,
        6460395907742683217LLU,
        12977000670426590916LLU,
        16860376138906378975LLU,
        16222164712469100257LLU,
        15007217811426180703LLU,
        11606636602190298050LLU,
        5561177516030672686LLU,
        2657178029372805182LLU,
        16009993004451944250LLU,
        7035976906297835190LLU,
        5190975052461767228LLU,
        12143693349456769736LLU,
        6051136899133467856LLU,
        5645448585517400002LLU,
        18017465147998176018LLU,
        9635513767646835776LLU,
    };
};

#endif //BISHOPMAPGENERATOR_H
