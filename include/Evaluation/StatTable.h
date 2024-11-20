//
// Created by Jlisowskyy on 8/5/24.
//

#ifndef STAT_TABLE_H
#define STAT_TABLE_H

#include "../CompilationConstants.h"

#include <type_traits>
#include <algorithm>

template <typename PointLimitT, typename ScoreDownCoefT, size_t Dim = 1, size_t ...Args>
class StatTable
{
    using _innerTable_t = std::conditional_t<
        sizeof...(Args) == 0,
        int,
        StatTable<PointLimitT, ScoreDownCoefT, Args...>
    >;

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
    [[nodiscard]] INLINE int Get(const size_t idx, idxesT ...idxes) const
    {
        static_assert(sizeof...(idxes) == sizeof...(Args), "Mismatched dimensions");

        if constexpr (sizeof...(idxes) == 0)
            return _table[idx];
        else
            return _table[idx].Get(idxes...);
    }

    template<typename ...idxesT>
    INLINE void Set(const int value, const size_t idx, idxesT ...idxes)
    {
        static_assert(sizeof...(idxes) == sizeof...(Args), "Mismatched dimensions");

        if constexpr (sizeof...(idxes) == 0)
            _table[idx] = value;
        else
            _table[idx].Set(value, idxes...);
    }

    template<typename ...idxesT>
    INLINE void SetPoints(const int points, idxesT ...idxes)
    {
        static_assert(sizeof...(idxes) == sizeof...(Args) + 1, "Mismatched dimensions");
        const int clampedPoints = std::clamp(points, static_cast<int>(-PointLimitT::Get()), static_cast<int>(PointLimitT::Get()));

        const int entry = Get(idxes...);
        const int newValue = entry + clampedPoints - entry * abs(clampedPoints) / PointLimitT::Get();
        Set(newValue, idxes...);
    }

    INLINE void ClearTable()
    {
        for (_innerTable_t& innerObject : _table)
            if constexpr (sizeof...(Args) == 0)
                innerObject = 0;
            else
                innerObject.ClearTable();
    }

    INLINE void ScaleTableDown()
    {
        for (_innerTable_t& innerObject : _table)
            if constexpr (sizeof...(Args) == 0)
                innerObject /= ScoreDownCoefT::Get();
            else
                innerObject.ScaleTableDown();
    }

    template<typename... IdxesT>
    INLINE auto GetTable(const size_t idx, IdxesT... idxes)
    {
        if constexpr (sizeof...(idxes) == 0) {
            return &_table[idx];
        } else {
            return _table[idx].GetTable(idxes...);
        }
    }

private:
    // ------------------------------
    // Class fields
    // ------------------------------

    std::array<_innerTable_t, Dim> _table{};
};

#endif // STAT_TABLE_H
