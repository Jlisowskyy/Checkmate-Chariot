//
// Created by Jlisowskyy on 3/12/24.
//

#ifndef TRANSPOSITIONTABLES_H
#define TRANSPOSITIONTABLES_H

#include <cinttypes>
#include <climits>

#include "../MoveGeneration/Move.h"

/*
 *  Class represents transposition table which is used
 *  to store globally calculated position evaluation results and metadata.
 *  Improves general performance of ab procedure as well as improves parallelism implementation.
 *  It Allows not recalculating so-called 'transposition' positions, what means same positions that
 *  were obtained by different sequence of moves, which is quit frequent situation in chess.
 */

enum nodeType: uint8_t
{
    pvNode,
    lowerBound,
    upperBound
};

struct TranspositionTable
{
    // ------------------------------
    // Class inner type
    // ------------------------------

    /*
     *  IMPORTANT: the size of hash record should be any number that is power of 2,
     *  to allow fast hashing function to do its job here.
     */

    struct alignas(16) HashRecord {
        // ------------------------------
        // Class creation
        // ------------------------------

        HashRecord() = default;
        ~HashRecord() = default;

        HashRecord(const uint64_t hash, const PackedMove mv, const int eval, const int statVal, const int depth, const nodeType nType, const uint16_t age):
            _zobristHashAndAgePacked(_packHashAndAge(hash, age)),
            _madeMove(mv),
            _eval(static_cast<int16_t>(eval)),
            _value(static_cast<int16_t>(statVal)),
            _depth(static_cast<uint8_t>(depth)),
            _type(nType)
        {}

        HashRecord(const HashRecord&) = default;
        HashRecord(HashRecord&&) = default;

        HashRecord& operator=(const HashRecord&) = default;
        HashRecord& operator=(HashRecord&&) = default;

        // ------------------------------
        // Class interaction
        // ------------------------------

        [[nodiscard]] PackedMove GetMove() const { return _madeMove; }
        [[nodiscard]] int GetEval() const { return _eval; }
        [[nodiscard]] int GetStatVal() const { return _value; }
        [[nodiscard]] int GetDepth() const { return  _depth; }
        [[nodiscard]] bool IsEmpty() const { return GetAge() == 0; }
        [[nodiscard]] uint16_t GetAge() const { return _zobristHashAndAgePacked & AgeBytes; }
        [[nodiscard]] nodeType GetNodeType() const { return _type; }
        [[nodiscard]] bool IsSameHash(const uint64_t hash) const { return (hash & HashBytes) == (_zobristHashAndAgePacked & HashBytes); }

        void SetStatVal(const int statEval) { _value = static_cast<int16_t>(statEval); }
        void SetEvalVal(const int eval) { _eval = static_cast<int16_t>(eval); }



        // ------------------------------
        // Private class methods
        // ------------------------------
    private:

        static uint64_t _packHashAndAge(const uint64_t Hash, const uint16_t age)
        {
            return (Hash & HashBytes) | age;
        }

        // ------------------------------
        // Class fields
        // ------------------------------
    public:

        static constexpr int NoEval = INT16_MAX;

    private:
        uint64_t _zobristHashAndAgePacked; // 6 bytes for valuable hash part and 2 bytes for age
        PackedMove _madeMove; // 2 Bytes
        int16_t _eval; // 2 bytes
        int16_t _value; // 2 bytes
        uint8_t _depth; // 1 byte
        nodeType _type; // 1 byte

        static constexpr uint64_t AgeBytes = 0xFFFF;
        static constexpr uint64_t HashBytes = ~AgeBytes;

        friend TranspositionTable;
    };

    struct alignas(32) TTBucket {

        enum class HitType {
            NoHit = -1,
            Hit0 = 0,
            Hit1 = 1
        };

        // ------------------------------
        // Class creation
        // ------------------------------

        TTBucket() = default;
        ~TTBucket() = default;

        // ------------------------------
        // Class interaction
        // ------------------------------

        [[nodiscard]] HitType IsHit(const uint64_t hash) const {
            if (_records[0].IsSameHash(hash)) return HitType::Hit0;
            if (_records[1].IsSameHash(hash)) return HitType::Hit1;
            return HitType::NoHit;
        }

        HashRecord& GetRecord(HitType type) {
            return type == HitType::Hit0 ? _records[0] : _records[1];
        }

        size_t GetSaveInfo(const nodeType type, const int depth, const int age)  const{
            const int bonusPv = (type == pvNode) * PvBonus;
            const int bonusLowerBound = (type == upperBound) * UpperBoundBonus;
            const int bonus = bonusPv + bonusLowerBound;

            int bestEv = INT_MAX;
            size_t ind = SIZE_MAX;

            for (size_t i = 0 ; i < BucketSize ; ++i) {
                const int ev = (depth - _records[i].GetDepth()) + bonus +
                    std::min(0, (age - _records[i].GetAge() - AgeConsideration))/2;

                if (ev > bestEv) {
                    bestEv = ev;
                    ind = i;
                }
            }

            return ind;
        }

        void Save(const HashRecord& record, size_t ind) {
            _records[ind] = record;
        }

        // ------------------------------
        // Private class methods
        // ------------------------------

        // ------------------------------
        // Class fields
        // ------------------------------

        static constexpr int16_t PvBonus = 3;
        static constexpr int16_t UpperBoundBonus = -1;
        static constexpr size_t BucketSize = 2;
        static constexpr int16_t AgeConsideration = 6;
        static constexpr size_t NOT_SAVABLE = SIZE_MAX;

    private:
        HashRecord _records[BucketSize]{};
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

    [[nodiscard]] TTBucket& __attribute__((always_inline)) GetBucket(const uint64_t zHash)
    {
        const size_t pos = zHash & _hashMask;
        return _map[pos];
    }

    void __attribute__((always_inline)) Prefetch(const uint64_t zHash) const
    {
        const size_t pos =  zHash & _hashMask;
        __builtin_prefetch(static_cast<const void *>(_map + pos));
    }

    void ClearTable();

    // IMPORTANT: function should only be used before any search was concluded, because is fully cleared when resizing
    ssize_t ResizeTable(size_t sizeMB);

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
    static constexpr size_t StartTableSize = StartTableSizeMB * MB / sizeof(TTBucket);

private:

    static constexpr size_t _entryAlignment = sizeof(TTBucket);

    size_t _tableSize{};
    size_t _hashMask{};
    TTBucket* _map{};
};

extern TranspositionTable TTable;

#endif  // TRANSPOSITIONTABLES_H
