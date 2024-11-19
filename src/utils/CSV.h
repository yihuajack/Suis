//
// Created by Yihua Liu on 2024/11/19.
//

#ifndef CSV_H
#define CSV_H

#include <string>
#include <vector>

namespace Utils {
    class CSV {
    public:
        static std::vector<std::vector<std::string>> readCSV(const std::string &filename);
        static void writeCSV(const std::string &filename, const std::vector<std::vector<std::string>> &data);
        static void modifyCell(std::vector<std::vector<std::string>> &data, size_t row, size_t col, const double newNum);
    };
}

#endif  // CSV_H
