//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVE_H
#define MOVE_H

#include <cinttypes>
#include <array>

#include "../EngineTypeDefs.h"

/*      Class encodes chess move and heuristic evaluation
 *  togethr inside some uint64_t value. Encoding corresponds to value below:
 *  (bit indexed from lsb to msb manner)
 *  - bits 0-16 encodes heristic evaluation of current move (16 bits)
 *                      Note: additionaly to allow simple evaluation on run
 *  - bits 16-22 - encodes field from which figure moved (6 bits)                                                       - "StartField" - OBLIG
 *                      IMPORTANT: should always be filled
 *  - bits 22-26 - encodes board index from which figure moved (4 bits)                                                 - "StartBoardIndex" - OBLIG
 *                      IMPORTANT: should always be filled
 *  - bits 26-32 - encodes field to which figure moved (6 bits)                                                         - "TargetField" - OBLIG
 *                      IMPORTANT: should always be filled
 *  - bits 32-36 - encodes board index to which figure moved - diffrent than option 2
 *          only in case move encodes promotion (4 bits)                                                                - "TargetBoardIndex" - OBLIG
 *                      IMPORTANT: should always be filled, in case no promotion is done simply put here positoin no 3
 *  - bits 36-40 - encodes board index on which figure was killed (4 bits) - used in case of
 *          attacking moves (4 bits)                                                                                    - "KilledFigureBoardIndex" - OBLIG
 *                      IMPORTANT: if not used should be filled with sentinel value
 *  - bits 40-46 - encodes field on which figure was killed - used only in case of el passant killing move (6 bits)     - "KilledFigureField" - OPT
 *                      WARNING: possibly unfilled when no attacking move and sentinel value is set
 *  - bits 46-52 - encodes new elPassant field (6 bits)                                                                 - "ElPassantField" - OBLIG
 *                      IMPORTANT: should always contain new el passant field, if not used simply put here literal from board
 *  - bits 52-56 - encodes castling rights (4 bits)                                                                     - "CastlingRights" - OLBIG
 *                      IMPORTANT: should always be filled, in case no changes was done simply copy previous values
 *  - bits 56-59 - encodes type of performed castlins (3 bits)                                                          - "CastlingType" - OPT
 *                      WARNING: possibly unfilled when no castling is done
 */

/*      IMPORTANT NOTE:
 *  ALL SET METHODS WORKS CORRECTLY ONLY
 *  ON BY DEFAULT INITIALIZED OBJECTS EVERY ONE OF THEM WORKS ONCE
 */

class Move
{
public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Move() = default;
    ~Move() = default;

    Move(const Move& other) = default;
    Move& operator=(const Move& other) = default;

    Move(Move&& other) = default;
    Move& operator=(Move&& other) = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] std::string GetLongAlgebraicNotation() const
    {
        static constexpr std::string FigTypeMap[] = {"", "n", "b", "r", "q"};
        std::string promotionMark;

        if (GetStartBoardIndex() != GetTargetBoardIndex())
            promotionMark = FigTypeMap[GetTargetBoardIndex() % Board::BoardsPerCol];

        return fieldStrMap.at(static_cast<Field>(maxMsbPossible >> GetStartField())) +
            fieldStrMap.at(static_cast<Field>(maxMsbPossible >> GetTargetField())) + promotionMark;
    }

    static void MakeMove(const Move mv, Board& bd)
    {
        // removing old piece from board
        bd.boards[mv.GetStartBoardIndex()] ^= maxMsbPossible >> mv.GetStartField();

        // placing figure on new field
        bd.boards[mv.GetTargetBoardIndex()] |= maxMsbPossible >> mv.GetTargetField();

        // removing killed figure in case no figure is killed index should be indicatin to the sentinel
        bd.boards[mv.GetKilledBoardIndex()] ^= maxMsbPossible >> mv.GetKilledFigureField();

        // applying new castling rights
        bd.Castlings = mv.GetCastlingRights();

        // applying new el passant field
        bd.elPassantField = maxMsbPossible >> mv.GetElPassantField();

        // applying addidtional castling operation
        auto [boardIndex, field] = CastlingActions[mv.GetCastlingType()];
        bd.boards[boardIndex] |= field;

        bd.ChangePlayingColor();
    }

    static void UnmakeMove(const Move mv, Board& bd, const std::array<bool, Board::CastlingCount+1>& castlings, const uint64_t oldElPassant)
    {
        bd.ChangePlayingColor();

        // placing piece on old board
        bd.boards[mv.GetStartBoardIndex()] |= maxMsbPossible >> mv.GetStartField();

        // removing figure from new field
        bd.boards[mv.GetTargetBoardIndex()] ^= maxMsbPossible >> mv.GetTargetField();

        // placing killed figure in good place
        bd.boards[mv.GetKilledBoardIndex()] |= maxMsbPossible >> mv.GetKilledFigureField();

        // recovering old castlings
        bd.Castlings = castlings;

        // recovering old el passant field
        bd.elPassantField = oldElPassant;

        // reverting castling operation
        auto [boardIndex, field] = CastlingActions[mv.GetCastlingType()];
        bd.boards[boardIndex] ^= field;
    }

    void SetEval(const int16_t eval)
    {
        _storage |= eval;
    }

    [[nodiscard]] int16_t GetEval() const
    {
        static constexpr uint64_t EvalMask = 0xFFFFLLU;
        return static_cast<int16_t>(_storage & EvalMask);
    }

    void SetStartField(const uint64_t startField)
    {
        _storage |= startField << 16;
    }

    [[nodiscard]]uint64_t GetStartField() const
    {
        static constexpr uint64_t StartFieldMask = 0x3FLLU << 16;
        return (_storage & StartFieldMask) >> 16;
    }

    void SetStartBoardIndex(const uint64_t startBoard)
    {
        _storage |= (startBoard << 22);
    }

    [[nodiscard]]uint64_t GetStartBoardIndex() const
    {
        static constexpr uint64_t StartBoardMask = 0xFLLU << 22;
        return (_storage & StartBoardMask) >> 22;
    }

    void SetTargetField(const uint64_t targetField)
    {
        _storage |= targetField << 26;
    }

    [[nodiscard]]uint64_t GetTargetField() const
    {
        static constexpr uint64_t TargetFieldMask = 0x3FLLU << 26;
        return (_storage & TargetFieldMask) >> 26;
    }

    void SetTargetBoardIndex(const uint64_t targetBoardIndex)
    {
        _storage |= targetBoardIndex << 32;
    }

    [[nodiscard]]uint64_t GetTargetBoardIndex() const
    {
        static constexpr uint64_t TargetBoardIndexMask = 0xFLLU << 32;
        return (_storage & TargetBoardIndexMask) >> 32;
    }

    void SetKilledBoardIndex(const uint64_t killedBoardIndex)
    {
        _storage |= killedBoardIndex << 36;
    }

    [[nodiscard]]uint64_t GetKilledBoardIndex() const
    {
        static constexpr uint64_t KilledBoardIndexMask = 0xFLLU << 36;
        return (_storage & KilledBoardIndexMask) >> 36;
    }

    void SetKilledFigureField(const uint64_t killedFigureField)
    {
        _storage |= killedFigureField << 40;
    }

    [[nodiscard]]uint64_t GetKilledFigureField() const
    {
        static constexpr uint64_t KilledFigureFieldMask = 0x3FLLU << 40;
        return (_storage & KilledFigureFieldMask) >> 40;
    }

    void SetElPassantField(const uint64_t elPassantField)
    {
        _storage |= elPassantField << 46;
    }

    [[nodiscard]]uint64_t GetElPassantField() const
    {
        static constexpr uint64_t ElPassantFieldMask = 0x3FLLU << 46;
        return (_storage & ElPassantFieldMask) >> 46;
    }

    void SetCasltingRights(const std::array<bool, Board::CastlingCount+1>& arr)
    {
        const uint64_t rights = (arr[0] << 0) | (arr[1] << 1) | (arr[2] << 2) | (arr[3] << 3);
        _storage |= rights << 52;
    }

    [[nodiscard]]std::array<bool, Board::CastlingCount+1> GetCastlingRights() const
    {
        static constexpr uint64_t CastlingMask = 0xFLLU << 52;
        std::array<bool, Board::CastlingCount+1> arr{};
        const uint64_t rights = (_storage & CastlingMask) >> 52;

        arr[0] = 1 & rights;
        arr[1] = (2 & rights) >> 1;
        arr[2] = (4 & rights) >> 2;
        arr[3] = (8 & rights) >> 3;

        return arr;
    }

    void SetCastlingType(const uint64_t castlingType)
    {
        _storage |= castlingType << 56;
    }

    [[nodiscard]]uint64_t GetCastlingType() const
    {
        static constexpr uint64_t CastlingTypeMask = 0x7LLU << 56;
        return (_storage & CastlingTypeMask) >> 56;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    static constexpr uint64_t Bit4 = 0b1111LLU;
    static constexpr uint64_t Bit6 = 0b111111LLU;
    static constexpr uint64_t Bit3 = 0b111LLU;

    uint64_t _storage{};

    static constexpr std::pair<size_t, uint64_t> CastlingActions[] = {
        { Board::SentinelBoardIndex, 0LLU },
        { wRooksIndex, Board::CastlingNewRookMaps[0] },
        { wRooksIndex, Board::CastlingNewRookMaps[1] },
        { bRooksIndex, Board::CastlingNewRookMaps[2] },
        { bRooksIndex, Board::CastlingNewRookMaps[3] },
    };
};

#endif //MOVE_H
