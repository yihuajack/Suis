cmake_minimum_required(VERSION 3.27)

if (CMAKE_HOST_WIN32)
    file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOTDIR)
    set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOTDIR}/scripts/buildsystems/vcpkg.cmake)
endif()

project(test-tmm-script)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

find_package(Boost 1.83.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories("../../src")

add_executable(test-tmm-script test_tmm_script.cpp)

target_link_libraries(test_tmm_script.cpp PRIVATE scsim_base)
add_test(NAME test_tmm_script.cpp COMMAND test_tmm_script.cpp)
add_dependencies(check test_tmm_script.cpp)
