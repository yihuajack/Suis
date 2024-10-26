#include <array>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>

// Function to read a CSV file into a 2D vector of strings
std::vector<std::vector<std::string>> readCSV(const std::string &filename) {
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
void writeCSV(const std::string &filename, const std::vector<std::vector<std::string>> &data) {
    std::ofstream file(filename);
    // Align with the first row
    std::size_t row_size = data.front().size();
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

void writeStats(const std::string &filename, const std::array<std::vector<double>, 4> &stats) {
    std::ofstream fout(filename);
    for (std::size_t j = 0; j < 8; j++) {
        for (std::size_t i = 0; i < 4; i++) {
            fout << stats.at(i).at(j) << ',';
        }
        fout << '\n';
    }
}

void readStats(const std::string &filename, std::array<std::vector<double>, 4> &stats) {
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

// Function to modify a specific cell in the CSV data
void modifyCell(std::vector<std::vector<std::string>> &data, size_t row, size_t col, const double newNum) {
    if (row < data.size() and col < data.at(row).size()) {
        std::ostringstream oss;
        oss << newNum << std::scientific;
        data.at(row).at(col) = oss.str();
    } else {
        std::cerr << "Invalid cell coordinates!\n";
    }
}

// Main function
int main() {
    const std::string filename = R"(E:\Documents\GitHub\ddmodel-octave\Input_files\spiro_mapi_tio2_custom.csv)";

    // Loop to modify the CSV file and run the external program
    const std::vector<double> possible_s1 = {1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8};
    const std::vector<double> possible_s2 = {1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9};
    const std::vector<double> possible_tau = {std::sqrt(10) * 1e-6, 1e-6, std::sqrt(10) * 1e-7, 1e-7, std::sqrt(10) * 1e-8, 1e-8, std::sqrt(10) * 1e-9, 1e-9};
    std::array<std::vector<double>, 4> stats;
    for (int i = 0; i < 8; ++i) {
        // Read the current state of the CSV file
        std::vector<std::vector<std::string>> data = readCSV(filename);

        // Modify a specific cell (example: 2nd row, 3rd column)
//        modifyCell(data, 5, 25, std::to_string(possible_s2.at(i)));  // ETL sp
//        modifyCell(data, 3, 24, std::to_string(possible_s1.at(i)));  // HTL sn
//        modifyCell(data, 5, 24, std::to_string(possible_s1.at(i)));  // ETL sn
//        modifyCell(data, 3, 25, std::to_string(possible_s1.at(i)));  // HTL sp
        modifyCell(data, 4, 22, possible_tau.at(i));  // tau_n
//        modifyCell(data, 4, 23, possible_tau.at(i));  // tau_p

        // Write the modified data back to the CSV
        writeCSV(filename, data);

        // Run the external program with the modified CSV file as an argument
        int result = std::system("matlab -batch \"run('E:/Documents/GitHub/ddmodel-octave/demo_ms_pin.m')\"");

        if (result) {
            std::cerr << "Error running MATLAB!\n";
            return 1;
        }

        readStats(R"(E:\Documents\GitHub\ddmodel-octave\stats.csv)", stats);

        std::filesystem::path newpath = R"(E:\Documents\Master\ECO\GCL\ParEngData\JV3-)" + std::to_string(i + 32) + ".csv";
        std::filesystem::rename(R"(E:\Documents\GitHub\ddmodel-octave\JV.csv)", newpath);
    }
    writeStats("4stats.csv", stats);
}
