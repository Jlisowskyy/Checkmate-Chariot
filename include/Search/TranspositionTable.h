//
// Created by Jlisowskyy on 3/12/24.
//

#ifndef TRANSPOSITIONTABLES_H
#define TRANSPOSITIONTABLES_H

#include <cinttypes>
#include <cstdlib>
#include <bit>

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
        uint64_t ZobristHash;
        uint8_t Depth;

    };

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

    void Add(const HashRecord& record);

    const HashRecord& GetRecord(uint64_t zHash)  const;

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
    size_t _containedRecords{};
    size_t _tableSize{};
    size_t _hashMaks{};
    HashRecord* _map{};
};

extern TranspositionTable TTable;

#endif  // TRANSPOSITIONTABLES_H
