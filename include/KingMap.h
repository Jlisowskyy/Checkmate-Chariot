//
// Created by Jlisowskyy on 12/30/23.
//

#ifndef KINGMAP_H
#define KINGMAP_H


#include <array>
#include <cstdint>

#include "EngineTypeDefs.h"
#include "movesHashMap.h"
#include "BitOperations.h"
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
    // Class creation
    // ------------------------------
public:
    constexpr KingMap() {
        movesMap = GenStaticMoves(maxMovesCount, movesCords, rowCords);
    }

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t, const uint64_t allyMap) const{
        const uint64_t moves = movesMap[msbInd];

        return ClearAFromIntersectingBits(moves, allyMap);
    }

    // ------------------------------
    // Class Fields
    // ------------------------------
private:

    std::array<uint64_t, Board::BoardFields> movesMap{};

    static constexpr size_t maxMovesCount = 8;

    // Describes king possible moves cordinates.
    static constexpr int movesCords[] = { -1, -9, -8, -7, 1, 9, 8, 7 };

    // Describes accordingly y positions after the move relatively to king's y position.
    // Used to omit errors during generation.
    static constexpr int rowCords[] = { 0, -1, -1, -1, 0, 1, 1, 1 };
};

#endif //KINGMAP_H
