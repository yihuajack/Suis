#include <fstream>
#include <iostream>
#include "tmm.h"

int main() {
    bool no_back_reflection = false;
    char pol = 's';
    bool coherent = true;
    std::complex<double> angle = 0;
    std::ifstream file("indices_0.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening the file." << std::endl;
        return 1;
    }
    std::vector<std::complex<double>> data;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        double real_part, imag_part;

        if (!(iss >> real_part >> imag_part)) {
            std::cerr << "Error parsing the line." << std::endl;
            return 1;
        }

        data.emplace_back(real_part, imag_part);
    }
}
