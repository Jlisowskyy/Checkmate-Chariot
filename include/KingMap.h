//
// Created by Jlisowskyy on 12/30/23.
//

#ifndef KINGMAP_H
#define KINGMAP_H


#include <array>
#include <cstdint>

#include "EngineTypeDefs.h"
#include "movesHashMap.h"

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
        movesMap = _genMoves();
    }

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] constexpr uint64_t GetMoves(int msbInd, uint64_t, uint64_t allyMap) const;

    // ------------------------------
    // private methods
    // ------------------------------
private:

    // Pregenerates all king moves on compilation stage.
    [[nodiscard]] static constexpr std::array<uint64_t, Board::BoardFields> _genMoves();

    // ------------------------------
    // Class Fields
    // ------------------------------

    std::array<uint64_t, Board::BoardFields> movesMap{};

    // Describes king possible moves cordinates.
    static constexpr int moveCords[] = { -1, -9, -8, -7, 1, 9, 8, 7 };

    // Describes accordingly y positions after the move relatively to king's y position.
    // Used to omit errors during generation.
    static constexpr int rowCords[] = { 0, -1, -1, -1, 0, 1, 1, 1 };
};

#endif //KINGMAP_H
