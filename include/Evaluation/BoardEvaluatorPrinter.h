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
    template <EvalMode mode>
    static void printBoardWithEval(const std::array<int16_t, 64> &eval)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            GlobalLogger.LogStream << std::string(89, '-') << std::endl;
            for (int i = 0; i < 8*2; ++i)
            {
                if(i%2==0)
                {
                    for (int j = 0; j < 8; ++j)
                    {
                        if(figureType[i/2*8+j]!=0)
                            GlobalLogger.LogStream << "|" << std::string(5, ' ')<< figureType[i/2*8+j] << std::string(4, ' ');
                        else
                            GlobalLogger.LogStream << "|" << std::string(10, ' ');
                    }
                    GlobalLogger.LogStream << "|" << std::endl;
                }
                else
                {
                    for (int j = 0; j < 8; ++j)
                    {
                        std::string points = std::to_string(eval[i/2 * 8 + j]);
                        int spaces=9 - points.size();
                        int spacesLeft=spaces/2;
                        GlobalLogger.LogStream << "| " << std::string(spacesLeft, ' ')<< points << std::string(spaces-spacesLeft, ' ');
                    }
                    GlobalLogger.LogStream << "|" << std::endl;
                    GlobalLogger.LogStream << std::string(89, '-') << std::endl;
                }

            }
            GlobalLogger.LogStream << std::string(89, '-') << std::endl;
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
            figureType = {};
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

    template <EvalMode mode> static void setValueOfPiecePosition(const int pieceIndex, const int16_t value,const char pieceType,const int16_t color)
    {
        if constexpr (mode == EvalMode::PrintMode)
        {
            if(color==WHITE)
            {
                positionValue[pieceIndex]=value;
                figureType[pieceIndex]=pieceType;
            }
            else
            {
                positionValue[pieceIndex]=value*-1;
                figureType[pieceIndex]=pieceType+32; // change to lowercase
            }
        }
    }



};

#endif // CHECKMATE_CHARIOT_BOARDEVALUATORPRINTER_H
