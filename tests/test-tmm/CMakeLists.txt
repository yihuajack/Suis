cmake_minimum_required(VERSION 3.22)
project(test-tmm)

set(CMAKE_CXX_STANDARD 23)

add_executable(test-tmm test_tmm.cpp ../../src/optics/tmm.cpp
        ../../src/optics/FixedMatrix.cpp
        ../../src/tools/utils.cpp
)

target_include_directories(test-tmm PRIVATE ${CMAKE_SOURCE_DIR}/../../src/optics ${CMAKE_SOURCE_DIR}/../../src/tools)

# For MSVC Compatibility
# See https://stackoverflow.com/questions/6563810/m-pi-works-with-math-h-but-not-with-cmath-in-visual-studio/6563891
# add_compile_definitions(_USE_MATH_DEFINES)
# Instead of using M_PI from <cmath> (included from <complex> for gcc/clang)
# We use std::numeric::pi and std::numeric::pi_v<T> from <numeric> since C++20 for better compatibility.
