//
// Created by Jlisowskyy on 12/30/23.
//

#ifndef KINGMAP_H
#define KINGMAP_H


#include <array>
#include <cstdint>

#include "../EngineTypeDefs.h"
#include "MoveGeneration.h"

/*              NOTES:
 *  In future there should be compared two options:
 *  - to use hashing here with prime numbers
 *  - to use hashing with LSB or MSB
 *
 *
 *              TESTED!
 */

class KingMap {
    // ------------------------------
    // Class inner types
    // ------------------------------
public:

    struct PinningMasks {
        PinningMasks() = default;
        PinningMasks(const PinningMasks&) = default;
        ~PinningMasks() = default;

        constexpr PinningMasks(const int bInd) {
            constexpr int northBarrier = 64;
            constexpr int southBarrier = -1;

            const int westBarrier = ((bInd / 8) * 8) - 1;
            const int eastBarrier = westBarrier + 9;

            masks[NorthMask] = GenMask(northBarrier, bInd, 8, std::less{});
            masks[SouthMask] = GenMask(southBarrier, bInd, -8, std::greater{});
            // masks[EastMask] = GenMask(eastBarrier, )
        }

        static constexpr size_t PinningMasksCount = 8;
        std::array<uint64_t, PinningMasksCount> masks{};

        enum PinnigMaskssNames{
            NorthMask,
            SouthMask,
            WestMask,
            EastMask,
            NorthWestMask,
            SouthEastMask,
            NorthEastMask,
            SouthWestMask,
        };
    };

    // ------------------------------
    // Class creation
    // ------------------------------

     KingMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd) {
        return movesMap[msbInd];
    }

    // ------------------------------
    // Class Fields
    // ------------------------------

    // static constexpr std::array<PinningMasks, Board::BoardFields> pinMasks =
private:

    static constexpr size_t maxMovesCount = 8;

    // Describes king possible moves cordinates.
    static constexpr int movesCords[] = { -1, -9, -8, -7, 1, 9, 8, 7 };

    // Describes accordingly y positions after the move relatively to king's y position.
    // Used to omit errors during generation.
    static constexpr int rowCords[] = { 0, -1, -1, -1, 0, 1, 1, 1 };

    static constexpr std::array<uint64_t, Board::BoardFields> movesMap = GenStaticMoves(maxMovesCount, movesCords, rowCords);
};

#endif //KINGMAP_H
