//
// Created by Jlisowskyy on 12/28/23.
//

#include "../include/EngineTypeDefs.h"

#include <format>

#include "../include/BitOperations.h"
#include "../include/Interface/Logger.h"
#include "../include/MoveGeneration/MoveGeneration.h"

std::ostream& operator<<(std::ostream&out, const Board&bd)
{
    std::string posBuffer = "00";

    for (size_t y = 0; y < 8; ++y)
    {
        for (size_t x = 0; x < 8; ++x)
        {
            posBuffer[0] = static_cast<char>(x + 'a');
            posBuffer[1] = static_cast<char>('8' - y);
            const uint64_t field = strFieldMap.at(posBuffer);

            GlobalLogger.StartLogging() << ' ';

            bool found = false;
            for (size_t desc = 0; desc < Board::BoardsCount; ++desc)
            {
                if ((bd.boards[desc] & field) != 0)
                {
                    GlobalLogger.StartLogging() << descToFigMap.at(desc);
                    found = true;
                    break;
                }
            }
            if (!found) GlobalLogger.StartLogging() << ' ';
            GlobalLogger.StartLogging() << ' ';

            if (x != 7) GlobalLogger.StartLogging() << '|';
            else GlobalLogger.StartLogging() << std::format("   {}", (char)('8' - y));
        }

        GlobalLogger.StartLogging() << std::endl;
        if (y != 7) GlobalLogger.StartLogging() << std::string(7 + 3 * 8, '-') << std::endl;
    }

    GlobalLogger.StartLogging() << std::string(7 + 3 * 8, '-') << std::endl;
    for (size_t x = 0; x < 8; ++x)
    {
        GlobalLogger.StartLogging() << ' ' << (char)('A' + x) << ' ' << ' ';
    }
    GlobalLogger.StartLogging() << std::endl;

    GlobalLogger.StartLogging() << "Moving color: " << (bd.movColor == WHITE ? "white" : "black") << std::endl;
    GlobalLogger.StartLogging() << "Possible castlings:\n";
    static constexpr const char* castlingNames[] =
            {"White King Side", "White Queen Side", "Black King Side", "Black Queen Side"};
    for (size_t i = 0; i < 4; ++i)
    {
        GlobalLogger.StartLogging() << castlingNames[i] << ": " << bd.Castlings[i] << std::endl;
    }

    GlobalLogger.StartLogging() << "El passand field: " << (bd.elPassantField == INVALID
                                                                ? "-"
                                                                : fieldStrMap.at(bd.elPassantField))
            << std::endl;

    return out;
}

void Board::ChangePlayingColor()
{
    movColor = SwapColor(movColor);
}

int SwapColor(const int col)
{
    return col ^ 1;
}

void DisplayMask(const uint64_t mask)
{
    for (int y = 56; y >= 0; y -= 8)
    {
        for (int x = 0; x < 8; ++x)
        {
            const uint64_t pos = 1LLU << (y + x);

            GlobalLogger.StartLogging() << ' ' << ((pos & mask) != 0 ? 'x' : ' ') << ' ' << (x != 7 ? '|' : '\n');
        }

        if (y != 0) GlobalLogger.StartLogging() << std::string(7 + 3 * 8, '-') << std::endl;
    }
}

std::tuple<uint64_t, uint64_t, MoveTypes> FindMove(const Board&oldBoard, const Board&newBoard)
{
    const size_t movingColInd = oldBoard.movColor * Board::BoardsPerCol;

    // its importtant search boards in reverse to firstly detect king moves as algebraic notation works
    for (ssize_t i = 5; i >= 0; --i)
    {
        if (oldBoard.boards[movingColInd + i] != newBoard.boards[movingColInd + i])
        {
            const uint64_t newPos = newBoard.boards[movingColInd + i] & ~oldBoard.boards[movingColInd + i];
            uint64_t oldPos = ~newBoard.boards[movingColInd + i] & oldBoard.boards[movingColInd + i];

            MoveTypes mType = NormalMove;

            // New figure was added to the board so the pawn was promoted
            if (oldPos == 0)
            {
                mType = PromotingMove;
                oldPos = ~newBoard.boards[movingColInd + pawnsIndex] & oldBoard.boards[movingColInd + pawnsIndex];
            }

            return {oldPos, newPos, mType};
        }
    }
#ifndef NDEBUG
    throw std::runtime_error("[ ERROR ] Move made not detected, malfunction found!");
#else
    return { 0, 0, NormalMove };
#endif
}

std::string GetEncodingFromBoards(const Board&bd, uint64_t oldMap, uint64_t newMap,
                                  MoveTypes mType)
{
    static constexpr std::string FigTypeMap[] = {"", "n", "b", "r", "q"};

    std::string figType = FigTypeMap[0];
    if (mType == PromotingMove)
    {
        const size_t searchIndex = SwapColor(bd.movColor) * Board::BoardsPerCol;

        for (size_t i = 1; i < 5; ++i)
            if ((bd.boards[searchIndex + i] & newMap) != 0)
            {
                figType = FigTypeMap[i];
                break;
            }
    }

    return fieldStrMap.at((Field)oldMap) + fieldStrMap.at((Field)newMap) + figType;
}

uint64_t ExtractPosFromStr(const int x, const int y)
{
    if (x > 'H' || x < 'A') return 0;
    if (y < '1' || y > '8') return 0;

    return 1LLU << ((y - '1') * 8 + (x - 'A'));
}

std::pair<uint64_t, uint64_t> ExtractPositionsFromEncoding(const std::string&encoding)
{
    if (encoding.length() < 4) return {0, 0};

    return {
        ExtractPosFromStr(std::toupper(encoding[0]), std::toupper(encoding[1])),
        ExtractPosFromStr(std::toupper(encoding[2]), std::toupper(encoding[3]))
    };
}

const std::unordered_map<size_t, Descriptors> figToDescMap{
    {'P', wPawnsIndex},
    {'N', wKnightsIndex},
    {'B', wBishopsIndex},
    {'R', wRooksIndex},
    {'Q', wQueensIndex},
    {'K', wKingIndex},
    {'p', bPawnsIndex},
    {'n', bKnightsIndex},
    {'b', bBishopsIndex},
    {'r', bRooksIndex},
    {'q', bQueensIndex},
    {'k', bKingIndex}
};

const std::unordered_map<size_t, char> descToFigMap{
    {wPawnsIndex, 'P'},
    {wKnightsIndex, 'N'},
    {wBishopsIndex, 'B'},
    {wRooksIndex, 'R'},
    {wQueensIndex, 'Q'},
    {wKingIndex, 'K'},
    {bPawnsIndex, 'p'},
    {bKnightsIndex, 'n'},
    {bBishopsIndex, 'b'},
    {bRooksIndex, 'r'},
    {bQueensIndex, 'q'},
    {bKingIndex, 'k'}
};

const std::unordered_map<Field, std::string> fieldStrMap{
    {INVALID, "XX"},
    {A1, "a1"},
    {B1, "b1"},
    {C1, "c1"},
    {D1, "d1"},
    {E1, "e1"},
    {F1, "f1"},
    {G1, "g1"},
    {H1, "h1"},
    {A2, "a2"},
    {B2, "b2"},
    {C2, "c2"},
    {D2, "d2"},
    {E2, "e2"},
    {F2, "f2"},
    {G2, "g2"},
    {H2, "h2"},
    {A3, "a3"},
    {B3, "b3"},
    {C3, "c3"},
    {D3, "d3"},
    {E3, "e3"},
    {F3, "f3"},
    {G3, "g3"},
    {H3, "h3"},
    {A4, "a4"},
    {B4, "b4"},
    {C4, "c4"},
    {D4, "d4"},
    {E4, "e4"},
    {F4, "f4"},
    {G4, "g4"},
    {H4, "h4"},
    {A5, "a5"},
    {B5, "b5"},
    {C5, "c5"},
    {D5, "d5"},
    {E5, "e5"},
    {F5, "f5"},
    {G5, "g5"},
    {H5, "h5"},
    {A6, "a6"},
    {B6, "b6"},
    {C6, "c6"},
    {D6, "d6"},
    {E6, "e6"},
    {F6, "f6"},
    {G6, "g6"},
    {H6, "h6"},
    {A7, "a7"},
    {B7, "b7"},
    {C7, "c7"},
    {D7, "d7"},
    {E7, "e7"},
    {F7, "f7"},
    {G7, "g7"},
    {H7, "h7"},
    {A8, "a8"},
    {B8, "b8"},
    {C8, "c8"},
    {D8, "d8"},
    {E8, "e8"},
    {F8, "f8"},
    {G8, "g8"},
    {H8, "h8"}
};

const std::unordered_map<std::string, Field> strFieldMap{
    {"a1", A1},
    {"b1", B1},
    {"c1", C1},
    {"d1", D1},
    {"e1", E1},
    {"f1", F1},
    {"g1", G1},
    {"h1", H1},
    {"a2", A2},
    {"b2", B2},
    {"c2", C2},
    {"d2", D2},
    {"e2", E2},
    {"f2", F2},
    {"g2", G2},
    {"h2", H2},
    {"a3", A3},
    {"b3", B3},
    {"c3", C3},
    {"d3", D3},
    {"e3", E3},
    {"f3", F3},
    {"g3", G3},
    {"h3", H3},
    {"a4", A4},
    {"b4", B4},
    {"c4", C4},
    {"d4", D4},
    {"e4", E4},
    {"f4", F4},
    {"g4", G4},
    {"h4", H4},
    {"a5", A5},
    {"b5", B5},
    {"c5", C5},
    {"d5", D5},
    {"e5", E5},
    {"f5", F5},
    {"g5", G5},
    {"h5", H5},
    {"a6", A6},
    {"b6", B6},
    {"c6", C6},
    {"d6", D6},
    {"e6", E6},
    {"f6", F6},
    {"g6", G6},
    {"h6", H6},
    {"a7", A7},
    {"b7", B7},
    {"c7", C7},
    {"d7", D7},
    {"e7", E7},
    {"f7", F7},
    {"g7", G7},
    {"h7", H7},
    {"a8", A8},
    {"b8", B8},
    {"c8", C8},
    {"d8", D8},
    {"e8", E8},
    {"f8", F8},
    {"g8", G8},
    {"h8", H8}
};

std::string GetShortAlgebraicMoveEncoding(const Board &oBoard, const Board &nBoard) {
    auto [oldBoard, newBoard, mType] = FindMove(oBoard, nBoard);
    return GetEncodingFromBoards(nBoard, oldBoard, newBoard, mType);
}
