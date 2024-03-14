//
// Created by Jlisowskyy on 3/13/24.
//

#include "../include/Search/TranspositionTable.h"

#include <stdexcept>
#include <format>
#include <cstring>
#include <cstdlib>

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

void TranspositionTable::Add(const HashRecord& record) {
    // TODO: adapt replacement policy

    const size_t pos =  record.ZobristHash & _hashMaks;
    _map[pos] = record;
}

TranspositionTable::HashRecord TranspositionTable::GetRecord(const uint64_t zHash) const {
    const size_t pos =  zHash & _hashMaks;
    return _map[pos];
}

void TranspositionTable::ClearTable() {
    memset(_map, 0, _tableSize * sizeof(int));
    _containedRecords = 0;
}

ssize_t TranspositionTable::ResizeTable(const size_t sizeMB)
{
    free(_map);
    const size_t ceiledSizeMB = std::bit_floor(sizeMB);
    const size_t objSize = ceiledSizeMB*MB/sizeof(HashRecord);
    _map = static_cast<HashRecord *>(std::aligned_alloc(_entryAlignment, StartTableSize*sizeof(HashRecord)));
    ClearTable();
    _containedRecords = 0;

    if (_map == nullptr)
    {
        _map = static_cast<HashRecord *>(std::aligned_alloc(_entryAlignment, StartTableSize*sizeof(HashRecord)));
        ClearTable();
        _tableSize = StartTableSize;
        _hashMaks = _getPow2ModuloMask(StartTableSize);
        _checkForCorrectAlloc(StartTableSize);
        return -1;
    }

    _tableSize = objSize;
    _hashMaks = _getPow2ModuloMask(objSize);
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
