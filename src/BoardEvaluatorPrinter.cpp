//
// Created by pietr on 19.05.2024.
//
#include "../include/Evaluation/BoardEvaluatorPrinter.h"

std::array<double, 64> BoardEvaluatorPrinter::positionValue = {};
std::array<char, 64> BoardEvaluatorPrinter::figureType = {};
std::array<double , 64> BoardEvaluatorPrinter::mobilityBonus = {};
std::array<double , 64> BoardEvaluatorPrinter::penaltyAndBonuses = {};
std::vector<std::string> BoardEvaluatorPrinter::additionalPoints={};
int BoardEvaluatorPrinter::positional=0;
int BoardEvaluatorPrinter::material=0;
int BoardEvaluatorPrinter::phase=0;