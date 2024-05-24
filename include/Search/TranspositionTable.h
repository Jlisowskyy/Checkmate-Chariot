//
// Created by Jlisowskyy on 3/12/24.
//

#ifndef TRANSPOSITIONTABLES_H
#define TRANSPOSITIONTABLES_H

#include <cinttypes>
#include <immintrin.h>

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"

/*
 *  Class represents transposition table which is used
 *  to store globally calculated position evaluation results and metadata.
 *  Improves general performance of ab procedure as well as improves parallelism implementation.
 *  It Allows not recalculating so-called 'transposition' positions, what means same positions that
 *  were obtained by different sequence of moves, which is quit frequent situation in chess.
 *
 *  Details:
 *  https://en.wikipedia.org/wiki/Transposition_table
 *  https://www.chessprogramming.org/Transposition_Table
 */

struct TranspositionTable
{
    // ------------------------------
    // Class inner type
    // ------------------------------

    /*
     *  IMPORTANT: the size of hash record should be any number that is power of 2,
     *  to allow fast hashing function to do its job here.
     */

    /*
     * The structure below stores all useful information about the position for the search algorithm.
     * Note that it takes only 48 bits of the hash value, and uses 16 bits for the age value.
     * It is done to reduce the size and the last 16 bits are mostly irrelevant because of the modulo operation.
     * It worth to say that minimal size set to be 16 MB (2^24 bytes / 2^4 bytes (size of the structure) ~= 2^20).
     * So the modulo value is always truncating at least 20 bits of the hash value.
     *
     * We are sure that those 20 bits are same when two distinct items land on the same index
     * */

    struct alignas(16) HashRecord
    {
        // ------------------------------
        // Class creation
        // ------------------------------

        HashRecord(
            const uint64_t hash, const PackedMove mv, const int eval, const int statVal, const int depth,
            const nodeType nType, const uint16_t age
        )
            : _zobristHashAndAgePacked(_packHashAndAge(hash, age)), _madeMove(mv), _eval(static_cast<int16_t>(eval)),
              _value(static_cast<int16_t>(statVal)), _depth(static_cast<uint8_t>(depth)), _type(nType)
        {
        }

        HashRecord(const HashRecord &) = default;
        HashRecord(HashRecord &&)      = default;

        HashRecord &operator=(const HashRecord &) = default;
        HashRecord &operator=(HashRecord &&)      = default;

        // ------------------------------
        // Class interaction
        // ------------------------------

        [[nodiscard]] PackedMove GetMove() const { return _madeMove; }
        [[nodiscard]] int GetEval() const { return _eval; }
        [[nodiscard]] int GetStatVal() const { return _value; }
        [[nodiscard]] int GetDepth() const { return _depth; }
        [[nodiscard]] bool IsEmpty() const { return GetAge() == 0; }
        [[nodiscard]] uint16_t GetAge() const { return _zobristHashAndAgePacked & AgeBytes; }
        [[nodiscard]] nodeType GetNodeType() const { return _type; }
        [[nodiscard]] bool IsSameHash(const uint64_t hash) const
        {
            // Note: Hash is packed together with age, so we need to compare only valuable part
            return (hash & HashBytes) == (_zobristHashAndAgePacked & HashBytes);
        }

        void SetStatVal(const int statEval) { _value = static_cast<int16_t>(statEval); }
        void SetEvalVal(const int eval) { _eval = static_cast<int16_t>(eval); }

        // ------------------------------
        // Private class methods
        // ------------------------------

        private:
        static uint64_t _packHashAndAge(const uint64_t Hash, const uint16_t age) { return (Hash & HashBytes) | age; }

        // ------------------------------
        // Class fields
        // ------------------------------

        public:
        static constexpr int NoEval = std::numeric_limits<int16_t>::max();

#ifdef NDEBUG // otherwise used inside the asserts out of the container

        private:
#endif

        uint64_t _zobristHashAndAgePacked; // 6 bytes for valuable hash part and 2 bytes for age
        PackedMove _madeMove;              // 2 Bytes
        int16_t _eval;                     // 2 bytes
        int16_t _value;                    // 2 bytes
        uint8_t _depth;                    // 1 byte
        nodeType _type;                    // 1 byte

        // Mask used to hash age
        static constexpr uint64_t AgeBytes = 0xFFFF;

        // Mask used to hash zobrist hash of the position
        static constexpr uint64_t HashBytes = ~AgeBytes;

        friend TranspositionTable;
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    TranspositionTable();

    ~TranspositionTable();

    TranspositionTable(const TranspositionTable &) = delete;

    TranspositionTable(TranspositionTable &&) = delete;

    TranspositionTable &operator=(const TranspositionTable &) = delete;

    TranspositionTable &operator=(TranspositionTable &&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // Method adds new record to the table
    INLINE void Add(const HashRecord &record, const uint64_t zHash)
    {
        TraceIfFalse((record.GetNodeType() == upperBound && record.GetMove().IsEmpty())
                || (record.GetNodeType() != upperBound && !record.GetMove().IsEmpty()),
                "Saved move is not valid!");

        // hash uses 48 bytes inside the record while masks uses at least log2(16 * 1024 * 1024 / 16) = 20
        const size_t pos = zHash & _hashMask;

        // if previously field was empty we need to increment the counter of contained records
        _containedRecords += _map[pos].IsEmpty();

        // save the given record
        _map[pos] = record;
    }

    // Methods retrieves record from the table
    [[nodiscard]] INLINE HashRecord &GetRecord(const uint64_t zHash) const
    {
        const size_t pos = zHash & _hashMask;
        return _map[pos];
    }

    // Method used to prefetch the record from the table short time before accessing it.
    // If we cannot find appropriate record this function becomes a noop.
    INLINE void Prefetch(const uint64_t zHash)
    {
        const size_t pos = zHash & _hashMask;

        // If we cannot find appropriate record this function becomes a noop
#ifdef __GNUC__
        __builtin_prefetch(static_cast<const void *>(_map + pos));
#elif defined(__SSE__)
        _mm_prefetch(static_cast<const void *>(_map + pos), _MM_HINT_T0);
#endif
    }

    void ClearTable();

    INLINE void UpdateStatistics(bool wasTTHit)
    {
        _hitsCount += wasTTHit;
        _missCount += !wasTTHit;
    }

    void DisplayStatisticsAndReset();

    // IMPORTANT: function should only be used before any search was concluded, because is fully cleared when resizing
    signed_size_t ResizeTable(size_t sizeMB);

    [[nodiscard]] size_t GetContainedElements() const;

    // ------------------------------
    // private methods
    // ------------------------------

    private:
    void _checkForCorrectAlloc(size_t size) const;

    static size_t _getPow2ModuloMask(const size_t pow2Num) { return pow2Num - 1; }

    // ------------------------------
    // Class fields
    // ------------------------------

    public:
    static constexpr size_t MaxSizeMB        = 1024 * 256;
    static constexpr size_t StartTableSizeMB = 16;
    static constexpr size_t MB               = 1024 * 1024;
    static constexpr size_t StartTableSize   = StartTableSizeMB * MB / sizeof(HashRecord);

    private:
    static constexpr size_t _entryAlignment = 16;

    size_t _containedRecords{};
    size_t _tableSize{};
    size_t _hashMask{};
    HashRecord *_map{};

    // used to gather statistics about the run
    uint64_t _hitsCount{};
    uint64_t _missCount{};
};

extern TranspositionTable TTable;

#endif // TRANSPOSITIONTABLES_H
