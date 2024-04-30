//
// Created by Jlisowskyy on 3/23/24.
//

#include "../include/Evaluation/KillerTable.h"

void KillerTable::ClearPlyFloor(const int depthLeft) {
    _kTable[depthLeft] = {};
}
