//
// Created by Jlisowskyy on 3/12/24.
//

#ifndef TRANSPOSITIONTABLES_H
#define TRANSPOSITIONTABLES_H

#include <cinttypes>
#include <cstdlib>

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

    TranspositionTable(): _map{static_cast<HashRecord *>(calloc(TableSize, sizeof(HashRecord)))} {
        _checkForCorrectAlloc();
    }

    ~TranspositionTable() {
        free(_map);
    }

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

    // ------------------------------
    // private methods
    // ------------------------------

    void _checkForCorrectAlloc() const;

    // ------------------------------
    // Class fields
    // ------------------------------

private:
    static constexpr size_t TableSizeGB = 16;
    static constexpr size_t GB = 1024 * 1024 * 1024;
    static constexpr size_t TableSize = TableSizeGB * GB / sizeof(HashRecord);

    HashRecord* _map{};
};

extern TranspositionTable TTable;

#endif  // TRANSPOSITIONTABLES_H
