cmake_minimum_required(VERSION 3.22)
project(tmm)

set(CMAKE_CXX_STANDARD 20)

add_executable(tmm test_tmm.cpp ../src/optics/tmm.cpp
        ../src/optics/FixedMatrix.cpp
        ../src/optics/utils.cpp)

target_include_directories(tmm PRIVATE ${CMAKE_SOURCE_DIR}/../src/optics)
