cmake_minimum_required(VERSION 3.0)
project(tmm)

set(CMAKE_CXX_STANDARD 20)

add_executable(tmm main.cpp ../../src/optics/tmm.cpp)
