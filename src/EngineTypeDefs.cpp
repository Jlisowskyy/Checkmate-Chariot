//
// Created by Jlisowskyy on 12/28/23.
//

#include "../include/EngineTypeDefs.h"

std::ostream& operator<<(std::ostream& out, const Board& bd) {
    std::string posBuffer = "00";

    for(size_t y = 0; y < 8; ++y){
        for(size_t x = 0; x < 8; ++x){
            posBuffer[0] = static_cast<char>(x + 'a');
            posBuffer[1] = static_cast<char>('8' - y);
            const uint64_t field = strFieldMap.at(posBuffer);

            std::cout << ' ';

            bool found = false;
            for (size_t desc = 0; desc < Board::BoardsCount; ++desc) {
                if ((bd.boards[desc] & field) != 0) {
                    std::cout << descToFigMap.at(desc);
                    found = true;
                    break;
                }
            }
            if (!found) std::cout << ' ';
            std::cout << ' ';

            if (x != 7) std::cout << '|';
        }

        std::cout << std::endl;
        if (y != 7) std::cout << std::string(7 + 3 * 8, '-') << std::endl;
    }

    std::cout << "Moving color: " << (bd.movColor == WHITE ? "white" : "black") << std::endl;

    std::cout << "Possible castlings:\n";
    static constexpr const char* castlingNames[] =
            { "White King Side", "White Queen Side", "Black King Side", "Black Queen Side"};
    for (size_t i = 0; i < 4; ++i) {
        std::cout << castlingNames[i] << ": " << bd.Castlings[i] << std::endl;
    }

    std::cout << "El passand field: " << (bd.elPassantField == INVALID ? "-" : fieldStrMap.at(bd.elPassantField))
            << std::endl;

    return out;
}

int SwapColor(const int col) {
    return 1 - col;
}

void DisplayMask(const uint64_t mask) {
    for (int y = 56; y >= 0; y-=8) {
        for (int x = 0; x < 8; ++x) {
            const uint64_t pos = 1LLU << (y + x);

            std::cout << ' ' << ((pos & mask) != 0 ? 'x' : ' ') << ' ' <<  (x != 7 ? '|' : '\n');
        }

        if (y != 0) std::cout << std::string(7 + 3 * 8, '-') << std::endl;
    }
}
