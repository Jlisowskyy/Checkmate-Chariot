//
// Created by Jlisowskyy on 4/29/24.
//

#include "../include/Evaluation/HistoricTable.h"

void HistoricTable::ClearTable()
{
    for (auto &table : _table) std::fill_n(table, Board::BitBoardFields, 0);
}

void HistoricTable::ScaleTableDown()
{
    for (auto &figureMap : _table)
        for (auto &field : figureMap) field /= ScaleFactor;

    _maxPoints /= ScaleFactor;
}

void HistoricTable::DisplayStats() const
{
    static constexpr size_t MAX_MOVES = 10;
    static constexpr int EMPTY_MOVE   = -Barrier - 1;

    std::tuple<std::tuple<size_t, size_t>, int> bestMoves[MAX_MOVES];
    const auto insertMove = [&](const size_t fig, const size_t field, const int points)
    {
        for (auto &move : bestMoves)
            if (points > std::get<1>(move))
            {
                move = {
                    {fig, field},
                    points
                };
                return;
            }
    };

    // init best moves
    for (auto &bestMove : bestMoves) bestMove = {{}, EMPTY_MOVE};

    int nonZeroElements{};
    int64_t sum{};
    int overflowsCounts{};

    // Collect statistics
    for (size_t figT = 0; figT < Board::BitBoardsCount; ++figT)
        for (size_t field = 0; field < Board::BitBoardFields; ++field)
        {
            const int points     = _getPoints(figT, field);
            const bool isNonZero = points != 0;

            if (isNonZero)
                insertMove(figT, field, points);

            nonZeroElements += isNonZero;
            sum += points;
            overflowsCounts += points == Barrier;
        }

    // calculate averages
    const double averageNonZero = static_cast<double>(sum) / static_cast<double>(nonZeroElements);
    const double averageOverall =
        static_cast<double>(sum) / static_cast<double>(Board::BitBoardsCount * Board::BitBoardFields);
    const int bestScore = std::get<1>(bestMoves[0]);

    GlobalLogger.LogStream << std::format(
        "Historic table statistics:\n"
        "\tNon zero average:    {}\n"
        "\tOverall average:     {}\n"
        "\tBest score:          {}\n"
        "\tNumber of overlows:  {}\n",
        averageNonZero, averageOverall, bestScore, overflowsCounts
    );

    // display collected Moves
    for (size_t i = 0; i < MAX_MOVES && std::get<1>(bestMoves[i]) != EMPTY_MOVE; ++i)
    {
        const auto [move, movePoints] = bestMoves[i];
        const auto [fig, field]       = move;

        GlobalLogger.LogStream << std::format("{}{}: {}\n", IndexToFigCharMap[fig], ConvertToStrPos(static_cast<int>(field)), movePoints);
    }

    GlobalLogger.LogStream << std::endl;
}
