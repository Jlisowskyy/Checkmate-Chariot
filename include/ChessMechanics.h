//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include "EngineTypeDefs.h"

class ChessMechanics {
    // ------------------------------
    // Class Creation
    // ------------------------------
public:

    explicit ChessMechanics(Board& bd): board(bd) {}

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] uint64_t GetBlockedMap()

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    Board& board;
};



#endif //CHESSMECHANICS_H
