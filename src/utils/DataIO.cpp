//
// Created by Yihua Liu on 2024/11/19.
//

#include <array>
#include <fstream>
#include <sstream>

#include "DataIO.h"

void Utils::DataIO::write4Stats(const std::string &filename, const std::array<std::vector<double>, 4> &stats) {
    std::ofstream fout(filename);
    for (std::size_t j = 0; j < 8; j++) {
        for (std::size_t i = 0; i < 4; i++) {
            fout << stats.at(i).at(j) << ',';
        }
        fout << '\n';
    }
}

std::array<std::vector<double>, 4> Utils::DataIO::read4Stats(const std::string &filename) {
    std::array<std::vector<double>, 4> stats;
    read4Stats(filename, stats);
    return stats;
}


void Utils::DataIO::read4Stats(const std::string &filename, std::array<std::vector<double>, 4> &stats) {
    std::ifstream file(filename);
    std::vector<std::vector<double>> data;
    std::string line, cell;

    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream lineStream(line);
        std::string metric, value;
        std::getline(lineStream, metric, ',');
        std::getline(lineStream, value);
        if (metric == "Jsc_r") {
            stats.front().emplace_back(std::stod(value));
        } else if (metric == "Voc_r") {
            stats.at(1).emplace_back(std::stod(value));
        } else if (metric == "FF_r") {
            stats.at(2).emplace_back(std::stod(value));
        } else if (metric == "efficiency_r") {
            stats.back().emplace_back(std::stod(value));
        }
    }
}

std::array<double, 13> Utils::DataIO::readSingleStats(const std::string &filename) {
    std::array<double, 13> stats{};
    std::ifstream file(filename);
    std::vector<std::vector<double>> data;
    std::string line, cell;
    std::size_t i = 0;

    while (std::getline(file, line)) {
        if (i >= 13) {
            break;
        }
        std::vector<std::string> row;
        std::stringstream lineStream(line);
        std::string metric, value;
        std::getline(lineStream, metric, ',');
        std::getline(lineStream, value);
        stats.at(i++) = std::stod(value);
    }
    return stats;
}
