//
// Created by Jlisowskyy on 3/13/24.
//

#include "../include/Search/TranspositionTable.h"

#include <stdexcept>
#include <format>
#include <cstring>
#include <cstdlib>
#include <bit>

#ifdef __AVX2__
#include <immintrin.h>
#endif // __AVX2__

TranspositionTable TTable{};

TranspositionTable::TranspositionTable():
    _tableSize(StartTableSize),
    _hashMaks(_getPow2ModuloMask(StartTableSize)),
    _map{static_cast<HashRecord *>(std::aligned_alloc(_entryAlignment, sizeof(HashRecord)*StartTableSize))}
{
    _checkForCorrectAlloc(StartTableSize);
    ClearTable();
}

TranspositionTable::~TranspositionTable()
{
    free(_map);
}

void TranspositionTable::Add(const HashRecord& record)
{
    // TODO: adapt replacement policy

    const size_t pos =  record._zobristHash & _hashMaks;
    _containedRecords += _map[pos].IsEmpty();
    _map[pos] = record;

// #ifdef __AVX__
//     // _mm256_stream_si256(reinterpret_cast<__m256i *>(_map + pos), *reinterpret_cast<const __m256i *>(&record));
//     _map[pos] = record;
//
// #else
//     _map[pos] = record;
// #endif
}

TranspositionTable::HashRecord TranspositionTable::GetRecord(const uint64_t zHash) const {
    const size_t pos =  zHash & _hashMaks;
    return _map[pos];


// #ifdef __AVX2__
//     __m256i loaded = _mm256_stream_load_si256(reinterpret_cast<__m256i *>(_map + pos));
//     void* rv = &loaded;
//     return *reinterpret_cast<HashRecord *>(rv);
//     // return _map[pos];
// #else
//     return _map[pos];
// #endif // __AVX2__
}

void TranspositionTable::Prefetch(const uint64_t zHash) const
{
    const size_t pos =  zHash & _hashMaks;
    __builtin_prefetch(static_cast<const void *>(_map + pos));
}

void TranspositionTable::ClearTable() {
    memset(_map, 0, _tableSize * sizeof(HashRecord));
    _containedRecords = 0;
}

ssize_t TranspositionTable::ResizeTable(const size_t sizeMB)
{
    free(_map);
    const size_t ceiledSizeMB = std::bit_floor(sizeMB);
    const size_t objSize = ceiledSizeMB*MB/sizeof(HashRecord);
    _map = static_cast<HashRecord *>(std::aligned_alloc(_entryAlignment, ceiledSizeMB*MB));

    if (_map == nullptr)
    {
        _map = static_cast<HashRecord *>(std::aligned_alloc(_entryAlignment, StartTableSize*sizeof(HashRecord)));
        _tableSize = StartTableSize;
        _hashMaks = _getPow2ModuloMask(StartTableSize);
        _checkForCorrectAlloc(StartTableSize);
        ClearTable();

        return -1;
    }

    _tableSize = objSize;
    _hashMaks = _getPow2ModuloMask(objSize);
    ClearTable();

    return static_cast<ssize_t>(ceiledSizeMB);
}

size_t TranspositionTable::GetContainedElements() const
{
    return _containedRecords;
}

void TranspositionTable::_checkForCorrectAlloc(const size_t size) const {
    if (_map == nullptr)
        throw std::runtime_error(std::format("[ ERROR ] Not able to allocate enough memory: {}MB",
            size * sizeof(HashRecord) / MB));
}
