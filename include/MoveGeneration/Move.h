//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVE_H
#define MOVE_H

#include <array>

#include "../Board.h"
#include "../Interface/Logger.h"

// TODO: repair description

/*      Class encodes chess move and heuristic evaluation
 *  together inside some uint64_t value. Encoding corresponds to value below:
 *  (bit indexed from lsb to msb manner)
 *  - bits 52-56 - encodes castling rights (4 bits) - "CastlingRights" - OBLIG IMPORTANT: should always be filled, in
 * case no changes was done simply copy previous values
 */

/*      Class encodes chess move and heuristic evaluation
 *  together inside some uint64_t value. Encoding corresponds to value below:
 *  - bits  0-15 - encodes simplistic heuristic evaluation of the move used inside inflight move sorting,
 *  - bits 16-31 - contains PackedMove instance
 *  - bits 32-35 - encodes board index from which figure moved
 *  - bits 36-39 - encodes board index to which figure moved - differs from StartBoardIndex only in case of promotion
 *  - bits 40-43 - encodes board index on which figure was killed used in case of attacking move
 *  - bits 44-46 - encodes type of performed castling e.g. white king-side castling
 *  - bit  47    - indicates whether given moves checks enemy king
 *  - bits 48-53 - encodes field on which figure was killed - differs from TargetField only in case of el passant
 *  - bits 54-59 - encodes new elPassant field
 *  - bits 60-63 - encodes new castling rights
 */

/*      IMPORTANT NOTE:
 *  ALL SET METHODS WORK CORRECTLY ONLY
 *  ON BY DEFAULT INITIALIZED OBJECTS EVERY ONE OF THEM WORKS ONCE
 */

class alignas(16) Move;

struct PackedMove
{
    // ------------------------------
    // Class creation
    // ------------------------------

    PackedMove()  = default;
    ~PackedMove() = default;

    PackedMove(const PackedMove &other)            = default;
    PackedMove &operator=(const PackedMove &other) = default;

    PackedMove(PackedMove &&other)            = default;
    PackedMove &operator=(PackedMove &&other) = default;

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

    [[nodiscard]] bool IsEmpty() const { return _packedMove == 0; }

    [[nodiscard]] bool IsQuiet() const { return (_packedMove & MoveTypeBits) == 0; }

    [[nodiscard]] bool IsCapture() const { return (_packedMove & CaptureBit) != 0; }

    [[nodiscard]] bool IsPromo() const { return (_packedMove & PromoBit) != 0; }

    [[nodiscard]] bool IsCastling() const { return (_packedMove & MoveTypeBits) == CastlingBits; }

    void SetMoveType(const uint16_t MoveType) { _packedMove |= MoveType << 12; }

    [[nodiscard]] uint16_t GetMoveType() const { return (_packedMove & MoveTypeBits) >> 12; }

    [[nodiscard]] bool IsValidMove() const { return !IsEmpty(); }

    // debugging tool
    [[nodiscard]] bool IsOkeyMove() const { return !IsEmpty() && GetTargetField() != GetStartField(); }

    [[nodiscard]] std::string GetLongAlgebraicNotation() const;

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint16_t PromoFlag     = 0b1000;
    static constexpr uint16_t CaptureFlag   = 0b100;
    static constexpr uint16_t CastlingFlag  = 0b10;
    static constexpr uint16_t QueenFlag     = 0;
    static constexpr uint16_t RookFlag      = 0b1;
    static constexpr uint16_t BishopFlag    = 0b10;
    static constexpr uint16_t KnightFlag    = 0b11;
    static constexpr uint16_t PromoSpecBits = 0b11;

    private:
    static constexpr uint16_t MoveTypeBits = 0xF << 12;
    static constexpr uint16_t Bit6         = 0b111111;

    static constexpr uint16_t PromoBit   = PromoFlag << 12;
    static constexpr uint16_t CaptureBit = CaptureFlag << 12;

    static constexpr uint16_t CastlingBits = CastlingFlag << 12;

    friend Move;

    uint16_t _packedMove;
};

/* Class used to preserve some crucial board state between moves */
struct VolatileBoardData
{
    VolatileBoardData() = delete;

    constexpr explicit VolatileBoardData(const Board &bd)
        : HalfMoves(bd.HalfMoves), Castlings(bd.Castlings), IsCheck(bd.IsCheck), OldElPassant(bd.ElPassantField)
    {
    }

    const int HalfMoves;
    const std::bitset<Board::CastlingCount + 1> Castlings;
    bool IsCheck;
    const uint64_t OldElPassant;
};

class Move
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    // This construction does not initialize crucial fields what must be done
    explicit Move(const PackedMove mv) : _packedMove(mv) {}

    Move()  = default;
    ~Move() = default;

    Move(const Move &other)            = default;
    Move &operator=(const Move &other) = default;

    Move(Move &&other)            = default;
    Move &operator=(Move &&other) = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    friend bool operator==(const Move a, const Move b) { return a._packedMove == b._packedMove; }
    friend bool operator!=(const Move a, const Move b) { return !(a == b); }

    [[nodiscard]] PackedMove GetPackedMove() const { return _packedMove; }

    void SetMoveType(const uint16_t MoveType) { _packedMove.SetMoveType(MoveType); }

    [[nodiscard]] uint16_t GetMoveType() const { return _packedMove.GetMoveType(); }

    [[nodiscard]] bool IsQuietMove() const { return !_packedMove.IsCapture() && !_packedMove.IsPromo(); }

    [[nodiscard]] bool IsValidMove() const { return _packedMove.IsValidMove(); }

    // debugging tool
    [[nodiscard]] bool IsOkeyMove() const { return _packedMove.IsOkeyMove(); }

    [[nodiscard]] std::string GetLongAlgebraicNotation() const { return _packedMove.GetLongAlgebraicNotation(); }

    static void MakeMove(const Move mv, Board &bd)
    {
        TraceIfFalse(mv.IsOkeyMove(), "Given move is not valid!");

        // removing the old piece from the board
        bd.BitBoards[mv.GetStartBoardIndex()] ^= MaxMsbPossible >> mv.GetStartField();

        // placing the figure on new field
        bd.BitBoards[mv.GetTargetBoardIndex()] |= MaxMsbPossible >> mv.GetTargetField();

        // removing the killed figure in case no figure is killed index should be indicating to the sentinel
        bd.BitBoards[mv.GetKilledBoardIndex()] ^= MaxMsbPossible >> mv.GetKilledFigureField();

        // applying new castling rights
        bd.Castlings = mv.GetCastlingRights();

        // applying new el passant field
        bd.ElPassantField = MaxMsbPossible >> mv.GetElPassantField();

        bd.HalfMoves = (bd.HalfMoves + 1) * (!(mv.GetStartBoardIndex() == wPawnsIndex ||
                                               mv.GetStartBoardIndex() == bPawnsIndex || mv.IsAttackingMove()));

        // applying additional castling operation
        const auto [boardIndex, field] = CastlingActions[mv.GetCastlingType()];
        bd.BitBoards[boardIndex] |= field;

        bd.ChangePlayingColor();

        bd.IsCheck = mv.IsChecking();
    }

    [[nodiscard]] bool IsAttackingMove() const { return _packedMove.IsCapture(); }

    [[nodiscard]] bool IsEmpty() const { return _packedMove.IsEmpty(); }

    static void UnmakeMove(const Move mv, Board &bd, const VolatileBoardData &data)
    {
        TraceIfFalse(mv.IsOkeyMove(), "Given move is not valid!");

        bd.ChangePlayingColor();

        // placing the piece on old board
        bd.BitBoards[mv.GetStartBoardIndex()] |= MaxMsbPossible >> mv.GetStartField();

        // removing the figure from the new field
        bd.BitBoards[mv.GetTargetBoardIndex()] ^= MaxMsbPossible >> mv.GetTargetField();

        // placing the killed figure in good place
        bd.BitBoards[mv.GetKilledBoardIndex()] |= MaxMsbPossible >> mv.GetKilledFigureField();

        // recovering old castlings
        bd.Castlings = data.Castlings;

        // recovering old el passant field
        bd.ElPassantField = data.OldElPassant;

        // Reversing Half Moves
        bd.HalfMoves = data.HalfMoves;

        // reverting castling operation
        const auto [boardIndex, field] = CastlingActions[mv.GetCastlingType()];
        bd.BitBoards[boardIndex] ^= field;

        bd.IsCheck = data.IsCheck;
    }

    void SetEval(const int32_t eval) { _eval = eval; }

    [[nodiscard]] int32_t GetEval() const { return _eval; }

    void SetStartField(const uint16_t startField) { _packedMove.SetStartField(startField); }

    [[nodiscard]] uint16_t GetStartField() const { return _packedMove.GetStartField(); }

    void SetTargetField(const uint16_t targetField) { _packedMove.SetTargetField(targetField); }

    [[nodiscard]] uint16_t GetTargetField() const { return _packedMove.GetTargetField(); }

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

    void SetCheckType()
    {
        static constexpr uint16_t CheckTypeBit = 1LLU << 15;
        _packedIndexes |= CheckTypeBit;
    }

    [[nodiscard]] bool IsChecking() const
    {
        static constexpr uint16_t CheckTypeBit = 1LLU << 15;
        return (_packedIndexes & CheckTypeBit) > 0;
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
        const uint16_t rights                  = (_packedMisc & CastlingMask) >> 12;
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

    int32_t _eval{};
    PackedMove _packedMove{};
    uint16_t _packedIndexes{};
    uint16_t _packedMisc{};

    public:
    static constexpr std::pair<size_t, uint64_t> CastlingActions[] = {
        {Board::SentinelBoardIndex,                          1LLU},
        {              wRooksIndex, Board::CastlingNewRookMaps[0]},
        {              wRooksIndex, Board::CastlingNewRookMaps[1]},
        {              bRooksIndex, Board::CastlingNewRookMaps[2]},
        {              bRooksIndex, Board::CastlingNewRookMaps[3]},
    };
};

#endif // MOVE_H
