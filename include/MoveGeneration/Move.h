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
 *  - bits 16-22 - encodes field from which figure moved (6 bits)
 *  - bits 22-26 - encodes board index from which figure moved (4 bits)
 *  - bits 26-32 - encodes field to which figure moved (6 bits)
 *  - bits 32-36 - encodes board index to which figure moved - diffrent than option 2
 *          only in case move encodes promotion (4 bits)
 *  - bits 36-40 - encodes board index on which figure was killed (4 bits) - used in case of
 *          attacking moves (4 bits)
 *  - bits 40-46 - encodes field on which figure was killed - used only in case of el passant killing move (6 bits)
 *  - bits 46-52 - encodes new elPassant field (6 bits)
 *  - bits 52-56 - encodes castling rights (4 bits)
 *
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

    // ------------------------------
    // Class interaction
    // ------------------------------

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
        bd.elPassantField = mv.GetElPassantField() == 0 ? INVALID : static_cast<Field>(maxMsbPossible >> mv.GetTargetField());
    }

    static void UnmakeMove(Move mv, const Board& bd)
    {

    }

    void SetEval(const int16_t eval)
    {
        _storage &= eval;
    }

    [[nodiscard]] int16_t GetEval() const
    {
        static constexpr uint64_t EvalMask = 0xFFFF;
        return _storage & EvalMask;
    }

    void SetStartField(const uint64_t startField)
    {
        _storage &= startField << 16;
    }

    [[nodiscard]]uint64_t GetStartField() const
    {
        static constexpr uint64_t StartFieldMask = 0x3F << 16;
        return (_storage & StartFieldMask) >> 16;
    }

    void SetStartBoardIndex(const uint64_t startBoard)
    {
        _storage &= (startBoard << 22);
    }

    [[nodiscard]]uint64_t GetStartBoardIndex() const
    {
        static constexpr uint64_t StartBoardMask = 0xF << 22;
        return (_storage & StartBoardMask) >> 22;
    }

    void SetTargetField(const uint64_t targetField)
    {
        _storage &= targetField << 26;
    }

    [[nodiscard]]uint64_t GetTargetField() const
    {
        static constexpr uint64_t TargetFieldMask = 0x3F << 26;
        return (_storage & TargetFieldMask) >> 26;
    }

    void SetTargetBoardIndex(const uint64_t targetBoardIndex)
    {
        _storage &= targetBoardIndex << 32;
    }

    [[nodiscard]]uint64_t GetTargetBoardIndex() const
    {
        static constexpr uint64_t TargetBoardIndexMask = 0xF << 32;
        return (_storage & TargetBoardIndexMask) >> 32;
    }

    void SetKilledBoardIndex(const uint64_t killedBoardIndex)
    {
        _storage &= killedBoardIndex << 36;
    }

    [[nodiscard]]uint64_t GetKilledBoardIndex() const
    {
        static constexpr uint64_t KilledBoardIndexMask = 0xF << 36;
        return (_storage & KilledBoardIndexMask) >> 36;
    }

    void SetKilledFigureField(const uint64_t killedFigureField)
    {
        _storage &= killedFigureField << 40;
    }

    [[nodiscard]]uint64_t GetKilledFigureField() const
    {
        static constexpr uint64_t KilledFigureFieldMask = 0x3F << 40;
        return (_storage & KilledFigureFieldMask) >> 40;
    }

    void SetElPassantField(const uint64_t elPassantField)
    {
        _storage &= elPassantField << 46;
    }

    [[nodiscard]]uint64_t GetElPassantField() const
    {
        static constexpr uint64_t ElPassantFieldMask = 0x3F << 46;
        return (_storage & ElPassantFieldMask) >> 46;
    }

    void SetCasltingRights(const std::array<bool, Board::CastlingCount>& arr)
    {
        const uint64_t rights = (arr[0] << 0) | (arr[1] << 1) | (arr[2] << 2) | (arr[3] << 3);
        _storage &= rights << 52;
    }

    [[nodiscard]]std::array<bool, Board::CastlingCount> GetCastlingRights() const
    {
        static constexpr uint64_t CastlingMask = 0xF << 52;
        std::array<bool, Board::CastlingCount> arr{};
        const uint64_t rights = (_storage & CastlingMask) >> 52;

        arr[0] = 1 & rights;
        arr[1] = (2 & rights) >> 1;
        arr[2] = (4 & rights) >> 2;
        arr[3] = (8 & rights) >> 3;

        return arr;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    uint64_t _storage{};
};

#endif //MOVE_H
