//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVE_H
#define MOVE_H

#include <array>

#include "../EngineTypeDefs.h"

// TOOD: repair description

/*      Class encodes chess move and heuristic evaluation
 *  togethr inside some uint64_t value. Encoding corresponds to value below:
 *  (bit indexed from lsb to msb manner)
 *  - bits 0-16 encodes heristic evaluation of current move (16 bits)
 *                      Note: additionaly to allow simple evaluation on run
 *  - bits 16-22 - encodes field from which figure moved (6 bits) - "StartField" - OBLIG IMPORTANT: should always be
 * filled
 *  - bits 22-26 - encodes board index from which figure moved (4 bits) - "StartBoardIndex" - OBLIG IMPORTANT: should
 * always be filled
 *  - bits 26-32 - encodes field to which figure moved (6 bits) - "TargetField" - OBLIG IMPORTANT: should always be
 * filled
 *  - bits 32-36 - encodes board index to which figure moved - diffrent than option 2
 *          only in case move encodes promotion (4 bits) - "TargetBoardIndex" - OBLIG IMPORTANT: should always be
 * filled, in case no promotion is done simply put here positoin no 3
 *  - bits 36-40 - encodes board index on which figure was killed (4 bits) - used in case of
 *          attacking moves (4 bits) - "KilledFigureBoardIndex" - OBLIG IMPORTANT: if not used should be filled with
 * sentinel value
 *  - bits 40-46 - encodes field on which figure was killed - used only in case of el passant killing move (6 bits) -
 * "KilledFigureField" - OPT WARNING: possibly unfilled when no attacking move and sentinel value is set
 *  - bits 46-52 - encodes new elPassant field (6 bits) - "ElPassantField" - OBLIG IMPORTANT: should always contain new
 * el passant field, if not used simply put here literal from board
 *  - bits 52-56 - encodes castling rights (4 bits) - "CastlingRights" - OLBIG IMPORTANT: should always be filled, in
 * case no changes was done simply copy previous values
 *  - bits 56-59 - encodes type of performed castlins (3 bits) - "CastlingType" - OPT WARNING: possibly unfilled when no
 * castling is done
 */

/*      IMPORTANT NOTE:
 *  ALL SET METHODS WORKS CORRECTLY ONLY
 *  ON BY DEFAULT INITIALIZED OBJECTS EVERY ONE OF THEM WORKS ONCE
 */

class Move;

struct PackedMove
{
    // ------------------------------
    // Class creation
    // ------------------------------

    PackedMove() = default;
    ~PackedMove() = default;

    PackedMove(const PackedMove& other) = default;
    PackedMove& operator=(const PackedMove& other) = default;

    PackedMove(PackedMove&& other) = default;
    PackedMove& operator=(PackedMove&& other) = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    friend bool operator==(const PackedMove a, const PackedMove b) { return a._packedMove == b._packedMove; }
    friend bool operator!=(const PackedMove a, const PackedMove b) { return !(a == b); }

    void SetStartField(const uint16_t startField) { _packedMove |= startField; }

    [[nodiscard]] uint16_t GetStartField() const
    {
        static constexpr uint16_t StartFieldMask = Bit6;
        return StartFieldMask & _packedMove;
    }

    void SetTargetField(const uint16_t targetField) { _packedMove |= targetField << 6; }

    [[nodiscard]] uint16_t GetTargetField() const
    {
        static constexpr uint16_t TargetFieldMask = Bit6 << 6;
        return (_packedMove & TargetFieldMask) >> 6;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return _packedMove == 0;
    }

    [[nodiscard]] bool IsQuiet() const
    {
        return (_packedMove & MoveTypeBits) == 0;
    }

    [[nodiscard]] bool IsCapture() const
    {
        return (_packedMove & CaptureBit) != 0;
    }

    [[nodiscard]] bool IsPromo() const
    {
        return (_packedMove & PromoBit) != 0;
    }

    void SetMoveType(const uint16_t MoveType)
    {
        _packedMove |= MoveType << 12;
    }

    [[nodiscard]] uint16_t GetMoveType() const
    {
        return (_packedMove & MoveTypeBits) >> 12;
    }

    [[nodiscard]] bool IsValidMove() const
    {
        return !IsEmpty();
    }

    // debugging tool
    [[nodiscard]] bool IsOkeyMove() const
    {
        return !IsEmpty() && GetTargetField() != GetStartField();
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint16_t PromoBit = 0x1 << 12;
    static constexpr uint16_t CaptureBit = 0x1 << 13;
    static constexpr uint16_t CastlingBit = 0x1 << 14;


private:
    static constexpr uint16_t MoveTypeBits = 0xF << 12;
    static constexpr uint16_t Bit6 = 0b111111;

    friend Move;

    uint16_t _packedMove;
};

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

    friend bool operator==(const Move a, const Move b) { return a._packedMove == b._packedMove; }
    friend bool operator!=(const Move a, const Move b) { return !(a == b); }

    [[nodiscard]] PackedMove GetPackedMove() const
    {
        return _packedMove;
    }

    void SetMoveType(const uint16_t MoveType)
    {
        _packedMove.SetMoveType(MoveType);
    }

    [[nodiscard]] uint16_t GetMoveType() const
    {
        return _packedMove.GetMoveType();
    }

    [[nodiscard]] bool IsQuietMove() const
    {
        return _packedMove.IsQuiet();
    }

    [[nodiscard]] bool IsValidMove() const
    {
        return _packedMove.IsValidMove();
    }

    // debugging tool
    [[nodiscard]] bool IsOkeyMove() const
    {
        return _packedMove.IsOkeyMove();
    }

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
        const auto [boardIndex, field] = CastlingActions[mv.GetCastlingType()];
        bd.boards[boardIndex] |= field;

        bd.ChangePlayingColor();
    }

    [[nodiscard]] bool IsAttackingMove() const
    {
        return _packedMove.IsCapture();
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return  _packedMove.IsEmpty();
    }

    static void UnmakeMove(const Move mv, Board& bd, const std::bitset<Board::CastlingCount + 1> castlings,
                           const uint64_t oldElPassant)
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
        const auto [boardIndex, field] = CastlingActions[mv.GetCastlingType()];
        bd.boards[boardIndex] ^= field;
    }

    void SetEval(const int16_t eval) { _eval = eval; }

    void ReplaceEval(const int16_t eval)
    {
        SetEval(eval);
    }

    [[nodiscard]] int16_t GetEval() const
    {
        return _eval;
    }

    void SetStartField(const uint16_t startField) { _packedMove.SetStartField(startField); }

    [[nodiscard]] uint16_t GetStartField() const
    {
        return _packedMove.GetStartField();
    }

    void SetTargetField(const uint16_t targetField) { _packedMove.SetTargetField(targetField); }

    [[nodiscard]] uint16_t GetTargetField() const
    {
        return _packedMove.GetTargetField();
    }

    void SetStartBoardIndex(const uint16_t startBoard) { _packedIndexes |= startBoard; }

    [[nodiscard]] uint16_t GetStartBoardIndex() const
    {
        static constexpr uint16_t StartBoardMask = Bit4;
        return _packedIndexes & StartBoardMask;
    }


    void SetTargetBoardIndex(const uint16_t targetBoardIndex) { _packedIndexes |= targetBoardIndex << 4; }

    [[nodiscard]] uint16_t GetTargetBoardIndex() const
    {
        static constexpr uint16_t TargetBoardIndexMask = Bit4 << 4;
        return (_packedIndexes & TargetBoardIndexMask) >> 4;
    }

    void SetKilledBoardIndex(const uint16_t killedBoardIndex) { _packedIndexes |= killedBoardIndex << 8; }

    [[nodiscard]] uint16_t GetKilledBoardIndex() const
    {
        static constexpr uint16_t KilledBoardIndexMask = Bit4 << 8;
        return (_packedIndexes & KilledBoardIndexMask) >> 8;
    }

    void SetCastlingType(const uint16_t castlingType) { _packedIndexes |= castlingType << 12; }

    [[nodiscard]] uint16_t GetCastlingType() const
    {
        static constexpr uint16_t CastlingTypeMask = Bit3 << 12;
        return (_packedIndexes & CastlingTypeMask) >> 12;
    }

    void SetKilledFigureField(const uint16_t killedFigureField) { _packedMisc |= killedFigureField; }

    [[nodiscard]] uint16_t GetKilledFigureField() const
    {
        static constexpr uint16_t KilledFigureFieldMask = Bit6;
        return _packedMisc & KilledFigureFieldMask;
    }

    void SetElPassantField(const uint16_t elPassantField) { _packedMisc |= elPassantField << 6; }

    [[nodiscard]] uint16_t GetElPassantField() const
    {
        static constexpr uint16_t ElPassantFieldMask = Bit6 << 6;
        return (_packedMisc & ElPassantFieldMask) >> 6;
    }

    void SetCasltingRights(const std::bitset<Board::CastlingCount + 1> arr)
    {
        const uint16_t rights = arr.to_ullong() & 0xFLLU;
        _packedMisc |= rights << 12;
    }

    [[nodiscard]] std::bitset<Board::CastlingCount + 1> GetCastlingRights() const
    {
        static constexpr uint16_t CastlingMask = Bit4 << 12;
        const uint16_t rights = (_packedMisc & CastlingMask) >> 12;
        const std::bitset<Board::CastlingCount + 1> arr{rights};

        return arr;
    }


    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    static constexpr uint16_t Bit4 = 0b1111;
    static constexpr uint16_t Bit6 = 0b111111;
    static constexpr uint16_t Bit3 = 0b111;

    int16_t _eval{};
    PackedMove _packedMove{};
    uint16_t _packedIndexes{};
    uint16_t _packedMisc{};
public:
    static constexpr std::pair<size_t, uint64_t> CastlingActions[] = {
        {Board::SentinelBoardIndex, 0LLU},
        {wRooksIndex, Board::CastlingNewRookMaps[0]},
        {wRooksIndex, Board::CastlingNewRookMaps[1]},
        {bRooksIndex, Board::CastlingNewRookMaps[2]},
        {bRooksIndex, Board::CastlingNewRookMaps[3]},
    };
};

#endif  // MOVE_H
