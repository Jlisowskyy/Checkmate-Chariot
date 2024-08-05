//
// Created by Jlisowskyy on 8/5/24.
//

#ifndef STATTABLE_H
#define STATTABLE_H
#include "../CompilationConstants.h"

template <typename PointLimitT, typename ScoreDownCoefT, size_t Dim, size_t ...Args>
class StatTable
{
    using _innerTable_t = StatTable<PointLimitT, ScoreDownCoefT, Args...>;

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
        const int clampedPoints = std::clamp(points, static_cast<int>(-PointLimitT::Get()), static_cast<int>(PointLimitT::Get()));

        const int entry = Get(idxes...);
        const int newValue = entry + clampedPoints - entry * abs(clampedPoints) / PointLimitT::Get();
        Set(newValue, idxes...);
    }

    INLINE void ClearTable()
    {
        for (_innerTable_t& table : _tables)
            table.ClearTable();
    }

    INLINE void ScaleTableDown()
    {
        for (_innerTable_t& table : _tables)
            table.ScaleTableDown();
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    _innerTable_t _tables[Dim]{};
};

template<typename PointLimitT, typename ScoreDownCoefT, size_t Dim>
class StatTable<PointLimitT, ScoreDownCoefT, Dim>
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

    INLINE void ClearTable()
    {
        std::fill(_table, _table + Dim, 0);
    }

    INLINE void ScaleTableDown()
    {
        for (auto& val : _table)
            val /= ScoreDownCoefT::Get();
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    int _table[Dim]{};
};

#endif //STATTABLE_H
