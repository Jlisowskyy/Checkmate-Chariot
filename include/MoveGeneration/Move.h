//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVE_H
#define MOVE_H

#include <array>

#include "../EngineTypeDefs.h"

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

    friend bool operator==(Move a, Move b);
    friend bool operator!=(Move a, Move b);

    [[nodiscard]] bool IsQuietMove() const;
    [[nodiscard]] std::string GetLongAlgebraicNotation() const;
    void MakeMove(Board& bd, std::bitset<Board::CastlingCount + 1> castlings, uint64_t oldElPassant) const;
    [[nodiscard]] bool IsAttackingMove() const;
    [[nodiscard]] bool IsEmpty() const;
    void UnmakeMove(Board& bd, std::bitset<Board::CastlingCount + 1> castlings, uint64_t oldElPassant) const;
    void SetEval(const int16_t eval) { _storage |= EvalMask & static_cast<uint64_t>(eval); }

    void ReplaceEval(int16_t eval);
    [[nodiscard]] int16_t GetEval() const;
    void SetStartField(uint64_t startField);
    [[nodiscard]] uint64_t GetStartField() const;
    void SetStartBoardIndex(uint64_t startBoard);
    [[nodiscard]] uint64_t GetStartBoardIndex() const;
    void SetTargetField(uint64_t targetField);
    [[nodiscard]] uint64_t GetTargetField() const;
    void SetTargetBoardIndex(uint64_t targetBoardIndex);
    [[nodiscard]] uint64_t GetTargetBoardIndex() const;
    void SetKilledBoardIndex(uint64_t killedBoardIndex);
    [[nodiscard]] uint64_t GetKilledBoardIndex() const;
    void SetKilledFigureField(uint64_t killedFigureField);
    [[nodiscard]] uint64_t GetKilledFigureField() const;
    void SetElPassantField(uint64_t elPassantField);
    [[nodiscard]] uint64_t GetElPassantField() const;
    void SetCasltingRights(std::bitset<Board::CastlingCount + 1> arr);
    [[nodiscard]] std::bitset<Board::CastlingCount + 1> GetCastlingRights() const;
    void SetCastlingType(uint64_t castlingType);
    [[nodiscard]] uint64_t GetCastlingType() const;

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint64_t EvalMask = 0xFFFFLLU;
    static constexpr uint64_t EvalMaskInverted = ~EvalMask;

private:
    static constexpr uint64_t Bit4 = 0b1111LLU;
    static constexpr uint64_t Bit6 = 0b111111LLU;
    static constexpr uint64_t Bit3 = 0b111LLU;

    uint64_t _storage{};

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
