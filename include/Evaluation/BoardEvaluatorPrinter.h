//
// Created by pietr on 19.05.2024.
//

#ifndef CHECKMATE_CHARIOT_BOARDEVALUATORPRINTER_H
#define CHECKMATE_CHARIOT_BOARDEVALUATORPRINTER_H

#include "../EngineUtils.h"
#include "../Interface/Logger.h"

class BoardEvaluatorPrinter
{
    static std::array<int16_t, 64> positionValue;
    static std::array<char, 64> figureType;

    private:
    template <EvalMode mode> static void printBoardWithEval(const std::array<int16_t, 64> &eval)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            const int fieldWidth  = 9;
            const int numRows     = 8;
            const int tableLength = fieldWidth * numRows + numRows + 1;

            GlobalLogger.LogStream << std::string(tableLength, '-') << std::endl;
            for (int i = 0; i < numRows; ++i)
            {
                // printing higher row
                for (int j = numRows - 1; j >= 0; --j)
                {
                    if (figureType[i * numRows + j] != 0)
                    {
                        int spaces     = fieldWidth - 1;
                        int spacesLeft = spaces / 2;
                        GlobalLogger.LogStream << "|" << std::string(spacesLeft, ' ') << figureType[i * numRows + j]
                                               << std::string(spaces - spacesLeft, ' ');
                    }
                    else
                        GlobalLogger.LogStream << "|" << std::string(fieldWidth, ' ');
                }
                GlobalLogger.LogStream << "| " << numRows - i << std::endl;

                // printing lower row
                for (int j = numRows - 1; j >= 0; --j)
                {
                    if (figureType[i * numRows + j] != 0)
                    {
                        std::string points =
                            ((eval[i * numRows + j] > 0) ? "+" : "") + std::to_string(eval[i * numRows + j]);
                        int spaces     = fieldWidth - points.length();
                        int spacesLeft = spaces / 2;
                        GlobalLogger.LogStream << "|" << std::string(spacesLeft, ' ') << points
                                               << std::string(spaces - spacesLeft, ' ');
                    }
                    else
                        GlobalLogger.LogStream << "|" << std::string(fieldWidth, ' ');
                }
                GlobalLogger.LogStream << "|" << std::endl;
                GlobalLogger.LogStream << std::string(tableLength, '-') << std::endl;
            }
            for (int x = 0; x < numRows; ++x)
            {
                int spaces     = fieldWidth;
                int spacesLeft = spaces / 2;
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
    template <EvalMode mode> static void resetEval()
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            positionValue = {};
            figureType    = {};
        }
    }

    // Function prints how evaluation works when EvalMode is PrintMode
    template <EvalMode mode> static void printAll()
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            printBoardWithEval<mode>(positionValue);
        }
    }

    template <EvalMode mode>
    static void
    setValueOfPiecePosition(const int pieceIndex, const int16_t value, const char pieceType, const int16_t color)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            if (color == WHITE)
            {
                positionValue[pieceIndex] = value;
                figureType[pieceIndex]    = pieceType;
            }
            else
            {
                positionValue[pieceIndex] = value * -1;
                figureType[pieceIndex]    = pieceType + 32; // change to lowercase
            }
        }
    }
};

#endif // CHECKMATE_CHARIOT_BOARDEVALUATORPRINTER_H
