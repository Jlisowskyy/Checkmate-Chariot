//
// Created by Jlisowskyy on 3/13/24.
//

#include "../include/Search/TranspositionTable.h"

#include <bit>
#include <cstdlib>
#include <cstring>
#include <format>
#include <stdexcept>

#include "../include/EngineUtils.h"

TranspositionTable TTable{};

TranspositionTable::TranspositionTable()
    : _tableSize(StartTableSize), _hashMask(_getPow2ModuloMask(StartTableSize)),
      _map{static_cast<HashRecord *>(AlignedAlloc(_entryAlignment, sizeof(HashRecord) * StartTableSize))}
{
    _checkForCorrectAlloc(StartTableSize);
    ClearTable();
}

TranspositionTable::~TranspositionTable() { AlignedFree(_map); }

void TranspositionTable::ClearTable()
{
    memset(_map, 0, _tableSize * sizeof(HashRecord));
    _containedRecords = 0;
}

signed_size_t TranspositionTable::ResizeTable(const size_t sizeMB)
{
    AlignedFree(_map);
    const size_t ceiledSizeMB = std::bit_floor(sizeMB);
    const size_t objSize      = ceiledSizeMB * MB / sizeof(HashRecord);
    _map                      = static_cast<HashRecord *>(AlignedAlloc(_entryAlignment, ceiledSizeMB * MB));

    if (_map == nullptr)
    {
        _map       = static_cast<HashRecord *>(AlignedAlloc(_entryAlignment, StartTableSize * sizeof(HashRecord)));
        _tableSize = StartTableSize;
        _hashMask  = _getPow2ModuloMask(StartTableSize);
        _checkForCorrectAlloc(StartTableSize);
        ClearTable();

        WrapTraceMsgError(std::format(
            "Not able to allocate enough memory for TTable, resizing to default size ({}MB)", StartTableSizeMB
        ));

        return -1;
    }

    _tableSize = objSize;
    _hashMask  = _getPow2ModuloMask(objSize);
    ClearTable();

    WrapTraceMsgInfo(std::format("TTable resized to {}MB", ceiledSizeMB));

    return static_cast<signed_size_t>(ceiledSizeMB);
}

size_t TranspositionTable::GetContainedElements() const { return _containedRecords; }

void TranspositionTable::_checkForCorrectAlloc(const size_t size) const
{
    if (_map == nullptr)
        throw std::runtime_error(
            std::format("[ ERROR ] Not able to allocate enough memory: {}MB", size * sizeof(HashRecord) / MB)
        );
}
