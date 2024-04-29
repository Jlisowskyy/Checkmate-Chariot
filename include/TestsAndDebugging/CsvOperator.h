//
// Created by Jlisowskyy on 3/3/24.
//

#ifndef CSVOPERATOR_H
#define CSVOPERATOR_H

#include <string>
#include <vector>

struct CsvOperator
{
    // ------------------------------
    // Class creation
    // ------------------------------

    CsvOperator()  = default;
    ~CsvOperator() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static std::vector<std::pair<std::string, int>> ReadPosDepthCsv(std::ifstream &stream);
    [[nodiscard]] static std::vector<std::pair<std::string, int>> ReadPosDepthCsv(const std::string &fileName);

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
};

#endif // CSVOPERATOR_H
