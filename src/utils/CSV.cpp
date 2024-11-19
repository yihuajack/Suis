//
// Created by Yihua Liu on 2024/11/19.
//

#include <fstream>
#include <iostream>
#include <sstream>

#include "CSV.h"

// Function to read a CSV file into a 2D vector of strings
std::vector<std::vector<std::string>> Utils::CSV::readCSV(const std::string &filename) {
    std::ifstream file(filename);
    std::vector<std::vector<std::string>> data;
    std::string line, cell;

    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream lineStream(line);
        while (std::getline(lineStream, cell, ',')) {
            row.push_back(cell);
        }
        data.push_back(row);
    }
    return data;
}

// Function to write a 2D vector of strings to a CSV file
void Utils::CSV::writeCSV(const std::string &filename, const std::vector<std::vector<std::string>> &data) {
    std::ofstream file(filename);
    // Align with the first row
    const std::size_t row_size = data.front().size();
    for (const std::vector<std::string> &row : data) {
        std::size_t i = 0;
        while (i < row.size()) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ',';
            }
            i++;
        }
        while (i < row_size) {
            file << ',';
            i++;
        }
        file << '\n';
    }
    // file.close();  The file will still be closed automatically when file goes out of scope.
}


// Function to modify a specific cell in the CSV data
void Utils::CSV::modifyCell(std::vector<std::vector<std::string>> &data, const size_t row, const size_t col, const double newNum) {
    if (row < data.size() and col < data.at(row).size()) {
        std::ostringstream oss;
        oss << newNum << std::scientific;
        data.at(row).at(col) = oss.str();
    } else {
        std::cerr << "Invalid cell coordinates!\n";
    }
}
