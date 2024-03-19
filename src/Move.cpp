//
// Created by Jlisowskyy on 3/19/24.
//

#include "../include/MoveGeneration/Move.h"
#include "../include/Search/ZobristHash.h"

bool Move::IsQuietMove() const
{
    // TODO: encode that during move gen?
    return GetKilledBoardIndex() == Board::SentinelBoardIndex && (GetStartBoardIndex() == GetTargetBoardIndex());
}

std::string Move::GetLongAlgebraicNotation() const
{
    static constexpr std::string FigTypeMap[] = {"", "n", "b", "r", "q"};
    std::string promotionMark;

    if (GetStartBoardIndex() != GetTargetBoardIndex())
        promotionMark = FigTypeMap[GetTargetBoardIndex() % Board::BoardsPerCol];

    return fieldStrMap.at(static_cast<Field>(maxMsbPossible >> GetStartField())) +
           fieldStrMap.at(static_cast<Field>(maxMsbPossible >> GetTargetField())) + promotionMark;
}

void Move::MakeMove(Board& bd, const std::bitset<Board::CastlingCount + 1> castlings,
    const uint64_t oldElPassant) const
{
    // removing old piece from board
    bd.boards[GetStartBoardIndex()] ^= maxMsbPossible >> GetStartField();

    // placing figure on new field
    bd.boards[GetTargetBoardIndex()] |= maxMsbPossible >> GetTargetField();

    // removing killed figure in case no figure is killed index should be indicatin to the sentinel
    bd.boards[GetKilledBoardIndex()] ^= maxMsbPossible >> GetKilledFigureField();

    // applying new castling rights
    bd.Castlings = GetCastlingRights();

    // applying new el passant field
    bd.elPassantField = maxMsbPossible >> GetElPassantField();

    // applying addidtional castling operation
    const auto [boardIndex, field] = CastlingActions[GetCastlingType()];
    bd.boards[boardIndex] |= field;

    bd.ChangePlayingColor();

    bd.zobristHash = ZHasher.UpdateHash(bd.zobristHash, *this, oldElPassant, castlings);
}

bool Move::IsAttackingMove() const
{
    return GetKilledBoardIndex() != Board::SentinelBoardIndex;
}

bool Move::IsEmpty() const
{
    return _storage == 0;
}

void Move::UnmakeMove(Board& bd, const std::bitset<Board::CastlingCount + 1> castlings,
    const uint64_t oldElPassant) const
{
    bd.ChangePlayingColor();

    // placing piece on old board
    bd.boards[GetStartBoardIndex()] |= maxMsbPossible >> GetStartField();

    // removing figure from new field
    bd.boards[GetTargetBoardIndex()] ^= maxMsbPossible >> GetTargetField();

    // placing killed figure in good place
    bd.boards[GetKilledBoardIndex()] |= maxMsbPossible >> GetKilledFigureField();

    // recovering old castlings
    bd.Castlings = castlings;

    // recovering old el passant field
    bd.elPassantField = oldElPassant;

    // reverting castling operation
    const auto [boardIndex, field] = CastlingActions[GetCastlingType()];
    bd.boards[boardIndex] ^= field;

    bd.zobristHash = ZHasher.UpdateHash(bd.zobristHash, *this, oldElPassant, castlings);
}

void Move::ReplaceEval(const int16_t eval)
{
    _storage = (_storage & EvalMaskInverted) | (EvalMask & static_cast<uint64_t>(eval));
}

int16_t Move::GetEval() const
{
    return static_cast<int16_t>(_storage & EvalMask);
}

void Move::SetStartField(const uint64_t startField)
{ _storage |= startField << 16; }

uint64_t Move::GetStartField() const
{
    static constexpr uint64_t StartFieldMask = 0x3FLLU << 16;
    return (_storage & StartFieldMask) >> 16;
}

void Move::SetStartBoardIndex(const uint64_t startBoard)
{ _storage |= (startBoard << 22); }

uint64_t Move::GetStartBoardIndex() const
{
    static constexpr uint64_t StartBoardMask = 0xFLLU << 22;
    return (_storage & StartBoardMask) >> 22;
}

void Move::SetTargetField(const uint64_t targetField)
{ _storage |= targetField << 26; }

uint64_t Move::GetTargetField() const
{
    static constexpr uint64_t TargetFieldMask = 0x3FLLU << 26;
    return (_storage & TargetFieldMask) >> 26;
}

void Move::SetTargetBoardIndex(const uint64_t targetBoardIndex)
{ _storage |= targetBoardIndex << 32; }

uint64_t Move::GetTargetBoardIndex() const
{
    static constexpr uint64_t TargetBoardIndexMask = 0xFLLU << 32;
    return (_storage & TargetBoardIndexMask) >> 32;
}

void Move::SetKilledBoardIndex(const uint64_t killedBoardIndex)
{ _storage |= killedBoardIndex << 36; }

uint64_t Move::GetKilledBoardIndex() const
{
    static constexpr uint64_t KilledBoardIndexMask = 0xFLLU << 36;
    return (_storage & KilledBoardIndexMask) >> 36;
}

void Move::SetKilledFigureField(const uint64_t killedFigureField)
{ _storage |= killedFigureField << 40; }

uint64_t Move::GetKilledFigureField() const
{
    static constexpr uint64_t KilledFigureFieldMask = 0x3FLLU << 40;
    return (_storage & KilledFigureFieldMask) >> 40;
}

void Move::SetElPassantField(const uint64_t elPassantField)
{ _storage |= elPassantField << 46; }

uint64_t Move::GetElPassantField() const
{
    static constexpr uint64_t ElPassantFieldMask = 0x3FLLU << 46;
    return (_storage & ElPassantFieldMask) >> 46;
}

void Move::SetCasltingRights(const std::bitset<Board::CastlingCount + 1> arr)
{
    const uint64_t rights = arr.to_ullong() & 0xFLLU;
    _storage |= rights << 52;
}

std::bitset<Board::CastlingCount + 1> Move::GetCastlingRights() const
{
    static constexpr uint64_t CastlingMask = 0xFLLU << 52;
    const uint64_t rights = (_storage & CastlingMask) >> 52;
    const std::bitset<Board::CastlingCount + 1> arr{rights};

    return arr;
}

void Move::SetCastlingType(const uint64_t castlingType)
{ _storage |= castlingType << 56; }

uint64_t Move::GetCastlingType() const
{
    static constexpr uint64_t CastlingTypeMask = 0x7LLU << 56;
    return (_storage & CastlingTypeMask) >> 56;
}

bool operator==(const Move a, const Move b)
{ return (Move::EvalMaskInverted & a._storage) == (Move::EvalMaskInverted& b._storage); }

bool operator!=(const Move a, const Move b)
{ return !(a == b); }
