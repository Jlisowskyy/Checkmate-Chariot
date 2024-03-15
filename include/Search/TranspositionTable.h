//
// Created by Jlisowskyy on 3/12/24.
//

#ifndef TRANSPOSITIONTABLES_H
#define TRANSPOSITIONTABLES_H

#include <cinttypes>
#include <cstdlib>
#include <climits>

#include "../MoveGeneration/Move.h"

/*
 *  Class represents transposition table which is used
 *  to store globally calculated position evaluation results and metadata.
 *  Improves general performance of ab procedure as well as improves paralellism implementation.
 *  Allows to not recalcullate so called 'transposition' positions, what means same position which
 *  were obtained by different sequence of moves, which is quit frequent situation in chess.
 */

struct TranspositionTable
{
    // ------------------------------
    // Class inner type
    // ------------------------------

    /*
     *  IMPORTANT size of hashrecord should be any number that is power of 2,
     *  to allow fast hashing function do its job here.
     */

    struct alignas(32) HashRecord {
        [[nodiscard]] uint64_t GetHash() const { return _zobristHash; }
        [[nodiscard]] Move GetMove() const { return _madeMove; }
        [[nodiscard]] int GetEval() const { return _eval; }
        [[nodiscard]] int GetStatVal() const { return _value; }
        [[nodiscard]] int GetDepth() const { return  _depth; }
        [[nodiscard]] bool IsPvNode() const { return (_type & PvNode) != 0; }

        HashRecord(const uint64_t hash, const Move mv, const int eval, const int statVal, const int depth, const uint8_t pvSpec):
            _zobristHash(hash),
            _madeMove(mv),
            _eval(eval),
            _value(statVal),
            _depth(depth)
        {
            _type |= pvSpec;
        }

        void SetStatVal(const int statEval) { _value = statEval; }
        void SetEvalVal(const int eval) { _eval = eval; }

        static constexpr int NoStatEval = INT_MAX;
        static constexpr int NoEval = INT_MAX;
        static constexpr uint8_t PvNode = 0x1;
        static constexpr uint8_t NoPvNode = 0;

    private:
        uint64_t _zobristHash; // 8 bytes
        Move _madeMove; // 8 Bytes
        int _eval; // 4 bytes
        int _value; // 4 bytes
        int _depth; // 4 byte
        // uint8_t _age; // 1 byte
        uint8_t _type{}; // 1 byte

        friend TranspositionTable;
    };

    // Total sum = 8 + 8 + 4 + 4 + 1 + 1 = 26 bytes => 6 byte padded <- try to improve memory management in future

    // ------------------------------
    // Class creation
    // ------------------------------

    TranspositionTable();

    ~TranspositionTable();

    TranspositionTable(const TranspositionTable&) = delete;

    TranspositionTable(TranspositionTable&&) = delete;

    TranspositionTable& operator=(const TranspositionTable&) = delete;

    TranspositionTable& operator=(TranspositionTable&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void Add(const HashRecord& record) const;

    [[nodiscard]] HashRecord GetRecord(uint64_t zHash)  const;

    void ClearTable();

    // IMPORTANT: function should only be used before any search was concluded, because is fully cleared when resizing
    ssize_t ResizeTable(size_t sizeMB);

    [[nodiscard]] size_t GetContainedElements() const;

    // ------------------------------
    // private methods
    // ------------------------------
private:
    void _checkForCorrectAlloc(size_t size) const;

    static size_t _getPow2ModuloMask(const size_t pow2Num)
    {
        return pow2Num - 1;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
public:

    static constexpr size_t MaxSizeMB = 1024 * 256;
    static constexpr size_t StartTableSizeMB = 16;
    static constexpr size_t MB = 1024 * 1024;
    static constexpr size_t StartTableSize = StartTableSizeMB * MB / sizeof(HashRecord);

private:

    static constexpr size_t _entryAlignment = 32;

    size_t _containedRecords{};
    size_t _tableSize{};
    size_t _hashMaks{};
    HashRecord* _map{};
};

extern TranspositionTable TTable;

#endif  // TRANSPOSITIONTABLES_H
