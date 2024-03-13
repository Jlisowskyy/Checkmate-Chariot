//
// Created by Jlisowskyy on 3/13/24.
//

#include "../include/Search/TranspositionTable.h"

#include <stdexcept>
#include <format>

TranspositionTable TTable{};

void TranspositionTable::Add(const HashRecord& record) {

}

const TranspositionTable::HashRecord& TranspositionTable::GetRecord(const uint64_t zHash) const {
    static HashRecord dummy;
    return dummy;
}

void TranspositionTable::ClearTable() {
    free(_map);
    _map = static_cast<HashRecord *>(calloc(TableSize, sizeof(HashRecord)));

    _checkForCorrectAlloc();
}

void TranspositionTable::_checkForCorrectAlloc() const {
    if (_map == nullptr)
        throw std::runtime_error(std::format("[ ERROR ] Not able to allocate enough memory: {}GB", TableSizeGB));
}
