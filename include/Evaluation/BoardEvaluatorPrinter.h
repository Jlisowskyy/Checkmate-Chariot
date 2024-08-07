//
// Created by pietraldo on 19.05.2024.
//

#ifndef CHECKMATE_CHARIOT_BOARDEVALUATORPRINTER_H
#define CHECKMATE_CHARIOT_BOARDEVALUATORPRINTER_H

#include "../EngineUtils.h"
#include "../Interface/Logger.h"
#include <vector>

inline INLINE double GetTapperedValuePrecise(int32_t phase, double midEval, double endEval)
{
    static constexpr double MaxTaperedCoef = 256; // HARD COPIED FROM BOARDEVALUATOR !!!
    return (endEval * (MaxTaperedCoef - phase) + midEval * phase) / MaxTaperedCoef;
}

class BoardEvaluatorPrinter
{
    // index 0 is a1 index 1 is b1
    static std::array<double, Board::BitBoardFields> positionValue;
    static std::array<char, Board::BitBoardFields> figureType;
    static std::array<double, Board::BitBoardFields> mobilityBonus;
    static std::array<double, Board::BitBoardFields> penaltyAndBonuses;
    static int phase;
    static int material;
    static int positional;
    static std::vector<std::string> additionalPoints;

    public:
    static INLINE double GetTapperedValue(double midEval, double endEval)
    {
        return GetTapperedValuePrecise(phase, midEval, endEval);
    }

    template <EvalMode mode> static void printBoardWithEval(const std::array<double, Board::BitBoardFields> &eval)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            const int fieldWidth  = 9;
            const int numRows     = 8;
            const int numCols     = 8;
            const int tableLength = fieldWidth * numRows + numRows + 1;

            GlobalLogger.LogStream << std::string(tableLength, '-') << std::endl;
            for (int i = numRows - 1; i >= 0; --i)
            {
                // printing higher row
                for (int j = 0; j < numCols; ++j)
                {
                    size_t spaces     = fieldWidth - 1;
                    size_t spacesLeft = spaces / 2;
                    GlobalLogger.LogStream << "|" << std::string(spacesLeft, ' ') << figureType[i * numCols + j]
                                           << std::string(spaces - spacesLeft, ' ');
                }
                GlobalLogger.LogStream << "| " << i << std::endl;

                // printing lower row
                for (int j = 0; j < numCols; ++j)
                {
                    if (figureType[i * numCols + j] != ' ')
                    {
                        std::string points =
                            ((eval[i * numCols + j] > 0) ? "+" : "") + std::format("{0:.1f}", eval[i * numCols + j]);
                        size_t spaces     = fieldWidth - points.length();
                        size_t spacesLeft = spaces / 2;
                        GlobalLogger.LogStream << "|" << std::string(spacesLeft, ' ') << points
                                               << std::string(spaces - spacesLeft, ' ');
                    }
                    else
                        GlobalLogger.LogStream << "|" << std::string(fieldWidth, ' ');
                }
                GlobalLogger.LogStream << "|" << std::endl;
                GlobalLogger.LogStream << std::string(tableLength, '-') << std::endl;
            }
            for (int x = 0; x < numCols; ++x)
            {
                size_t spaces     = fieldWidth;
                size_t spacesLeft = spaces / 2;
                GlobalLogger.LogStream << std::string(spacesLeft, ' ') << static_cast<char>('A' + x)
                                       << std::string(spaces - spacesLeft, ' ');
            }
            GlobalLogger.LogStream << std::endl << std::endl;
        }
    }

    public:
    // Function prints how evaluation works when EvalMode is PrintMode
    template <EvalMode mode> static void print(const std::string &str)
    {
        if constexpr (mode == EvalMode::PrintMode)
            GlobalLogger.LogStream << str;
    }

    // Function prints how evaluation works when EvalMode is PrintMode
    template <EvalMode mode> static void setBoard(const Board &board)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            figureType                         = {};
            static constexpr size_t FigsPerRow = 8;
            static constexpr size_t FigsPerCol = 8;
            for (size_t y = 0; y < FigsPerCol; ++y)
            {
                for (size_t x = 0; x < FigsPerRow; ++x)
                {
                    const uint64_t field = ExtractPosFromStr(static_cast<char>(x + 'a'), static_cast<char>('1' + y));

                    bool found = false;
                    for (size_t desc = 0; desc < Board::BitBoardsCount; ++desc)
                    {
                        if ((board.BitBoards[desc] & field) != 0)
                        {
                            figureType[y * FigsPerCol + x] = IndexToFigCharMap[desc];
                            found                          = true;
                            break;
                        }
                    }
                    if (!found)
                        figureType[y * FigsPerCol + x] = ' ';
                }
            }
        }
    }

    // Function prints how evaluation works when EvalMode is PrintMode
    template <EvalMode mode> static void resetEval()
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            positionValue     = {};
            figureType        = {};
            mobilityBonus     = {};
            penaltyAndBonuses = {};
            additionalPoints.clear();
        }
    }

    // Function prints how evaluation works when EvalMode is PrintMode
    template <EvalMode mode> static void printAll()
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            double all = 0;
            double sum = 0;
            for (int i = 0; i < 64; i++) sum += positionValue[i];
            GlobalLogger.LogStream << std::endl << "Position Values sum: " << sum << std::endl;
            printBoardWithEval<mode>(positionValue);
            all += sum;

            sum = 0;
            for (int i = 0; i < 64; i++) sum += mobilityBonus[i];
            GlobalLogger.LogStream << "Bonus Mobility: " << sum << std::endl;
            printBoardWithEval<mode>(mobilityBonus);
            all += sum;

            sum = 0;
            for (int i = 0; i < 64; i++) sum += penaltyAndBonuses[i];
            GlobalLogger.LogStream << "Penalty and Bonuses: " << sum << std::endl;
            printBoardWithEval<mode>(penaltyAndBonuses);
            all += sum;

            for (const auto &additionalPoint : additionalPoints) GlobalLogger.LogStream << additionalPoint;

            GlobalLogger.LogStream << std::endl;

            GlobalLogger.LogStream << "Phase: " << phase << std::endl;
            GlobalLogger.LogStream << "Material: " << material << std::endl;
            GlobalLogger.LogStream << "Positional: " << positional << std::endl;
            GlobalLogger.LogStream << "All without additional: " << all << std::endl;
        }
    }

    template <EvalMode mode> static void setAdditionalPoints(const std::string &points)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            additionalPoints.push_back(points);
        }
    }

    template <EvalMode mode> static void setPhase(const int value)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            phase = value;
        }
    }

    template <EvalMode mode> static void setPenaltyAndBonuses(const int pieceIndex, const int value)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            if (figureType[pieceIndex] < 'a') // WHITE
                penaltyAndBonuses[pieceIndex] += value;
            else
                penaltyAndBonuses[pieceIndex] += value * -1;
        }
    }

    template <EvalMode mode> static void setMaterial(const int value)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            material = value;
        }
    }

    template <EvalMode mode> static void setPositional(const int value)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            positional = value;
        }
    }

    template <EvalMode mode>
    static void setValueOfPiecePositionTappered(const int pieceIndex, const int midValue, const int endValue)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            const double value = GetTapperedValuePrecise(phase, midValue, endValue);
            if (figureType[pieceIndex] < 'a') // WHITE
                positionValue[pieceIndex] = value;
            else
                positionValue[pieceIndex] = value * -1;
        }
    }

    template <EvalMode mode>
    static void setPenaltyAndBonusesTapered(const int pieceIndex, const int midValue, const int endValue)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            const double value = GetTapperedValuePrecise(phase, midValue, endValue);
            if (figureType[pieceIndex] < 'a') // WHITE
                penaltyAndBonuses[pieceIndex] += value;
            else
                penaltyAndBonuses[pieceIndex] += value * -1;
        }
    }

    template <EvalMode mode>
    static void setMobilityBonusTappered(const int pieceIndex, const int midValue, const int endValue)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            const double value = GetTapperedValuePrecise(phase, midValue, endValue);
            if (figureType[pieceIndex] < 'a') // WHITE
                mobilityBonus[pieceIndex] = value;
            else
                mobilityBonus[pieceIndex] = value * -1;
        }
    }
};

#endif // CHECKMATE_CHARIOT_BOARDEVALUATORPRINTER_H
