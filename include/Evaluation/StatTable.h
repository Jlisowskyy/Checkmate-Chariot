//
// Created by Jlisowskyy on 8/5/24.
//

#ifndef STATTABLE_H
#define STATTABLE_H
#include "../CompilationConstants.h"

template <size_t dim, size_t ...args>
class StatTable
{
    using _innerTable_t = StatTable<args...>;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    StatTable() = default;
    ~StatTable() = default;

    StatTable(StatTable &&)      = delete;
    StatTable(const StatTable &) = delete;

    StatTable &operator=(const StatTable &) = delete;
    StatTable &operator=(StatTable &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template<typename ...idxesT>
    [[nodiscard]] INLINE int Get(const size_t idx, idxesT ...idxes) const { return _tables[idx].Get(idxes...); }

    template<typename ...idxesT>
    INLINE void Set(const int value, const size_t idx, idxesT ...idxes) { _tables[idx].Set(value, idxes...); }

    template<typename  ...idxesT>
    INLINE void SetPoints(const int points, idxesT ...idxes)
    {
        const int clampedPoints = std::clamp(points, (int)-HISTORY_TABLE_POINTS_LIMIT::Get(), (int)HISTORY_TABLE_POINTS_LIMIT::Get());

        const int entry = Get(idxes...);
        const int newValue = entry + clampedPoints - entry * abs(clampedPoints) / HISTORY_TABLE_POINTS_LIMIT::Get();
        Set(newValue, idxes...);
    }

    void ClearTable()
    {
        for (_innerTable_t& table : _tables)
            table.ClearTable();
    }

    void ScaleTableDown()
    {
        for (_innerTable_t& table : _tables)
            table.ScaleTableDown();
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    _innerTable_t _tables[dim]{};
};

template<size_t dim>
class StatTable<dim>
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    StatTable() = default;
    ~StatTable() = default;

    StatTable(StatTable &&)      = delete;
    StatTable(const StatTable &) = delete;

    StatTable &operator=(const StatTable &) = delete;
    StatTable &operator=(StatTable &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] INLINE int Get(const size_t idx) const { return _table[idx]; }
    INLINE void Set(const int value, const size_t idx) { _table[idx] = value; }

    void ClearTable()
    {
        std::fill(_table, _table + dim, 0);
    }

    void ScaleTableDown()
    {
        for (auto& val : _table)
            val /= HISTORY_SCALE_DOWN_FACTOR::Get();
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    int _table[dim]{};
};

#endif //STATTABLE_H
