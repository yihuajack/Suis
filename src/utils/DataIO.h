//
// Created by Yihua Liu on 2024/11/19.
//

#ifndef DATAIO_H
#define DATAIO_H

#include <string>
#include <vector>

namespace Utils {
    class DataIO {
    public:
        static void read4Stats(const std::string &filename, std::array<std::vector<double>, 4> &stats);
        static std::array<std::vector<double>, 4> read4Stats(const std::string &filename);
        static void write4Stats(const std::string &filename, const std::array<std::vector<double>, 4> &stats);
        static std::array<double, 13> readSingleStats(const std::string &filename);
    };
}

#endif  // DATAIO_H
