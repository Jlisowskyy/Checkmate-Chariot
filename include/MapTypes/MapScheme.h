//
// Created by Jlisowskyy on 2/6/24.
//

#ifndef MAPSCHEME_H
#define MAPSCHEME_H

#include <cinttypes>

struct MapScheme {
    virtual uint64_t GetMoves(int msbInd, uint64_t fullBoard);
};

inline uint64_t MapScheme::GetMoves(const int msbInd, const uint64_t fullBoard) {
}

#endif //MAPSCHEME_H
