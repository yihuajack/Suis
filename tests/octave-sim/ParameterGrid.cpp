#include <array>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>

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
        int result = std::system("matlab -batch \"run('E:/Documents/GitHub/ddmodel-octave/demo_ms_script.m')\"");

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
