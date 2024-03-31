//
// Created by Yihua Liu on 2024/03/28.
//

#include "Log.h"
#include <iostream>

template auto Utils::Log::complex_to_string_with_name(const std::complex<double> c, const std::string &name) -> std::string;

template<Utils::Log::TwoDContainer Container>
void Utils::Log::print_spec2d_container(const Container &container) {
    for (const auto &row: container) {
        for (const auto &item : row) {
            std::cout << item << ' ';
        }
        std::cout << '\n';
    }
}

// __GNUG__ is equivalent to (__GNUC__ && __cplusplus),
// see https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// See https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c
// https://stackoverflow.com/questions/13777681/demangling-in-msvc
#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
auto Utils::Log::demangle(const char *mangled_name) -> std::string {
    int status = 0;
    // Implicit conversion from char * to std::string.
    // If using char *, we need to manually free it, which is not RAII.
    std::string demangled_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
    if (status == 0) {
        std::string result(demangled_name);
        return result;
    }
    // Demangling failed, return the mangled name
    return mangled_name;
}
// Do not mix #if defined and #elifdef!
// Normally, `undname` command is only accessible under Developer Command Prompt/PowerShell for VS
#elif defined(_MSC_VER)
auto Utils::Log::demangle(const char *mangled_name) -> std::string {
    // Assemble the command to run the 'undname' utility
    std::string command = "undname -n\"" + std::string(mangled_name) + "\"";
    // Open a pipe to the command
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        return mangled_name; // Return the mangled name if the command fails
    }
    // Read the demangled name from the pipe
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    // Close the pipe
    _pclose(pipe);
    // Remove newline characters from the result
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}
#else
auto Utils::Log::demangle(const char *mangled_name) -> std::string {
    throw std::logic_error("Demangling only by gcc, clang, and msvc is supported.");
}
#endif

void Utils::Log::print_container(const std::ranges::common_range auto &container) {
    for (const auto &item: container) {
        std::cout << item << ' ';
    }
    std::cout << '\n';
}
