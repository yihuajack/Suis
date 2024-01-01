cmake_minimum_required(VERSION 3.22)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")  # == CMAKE_HOST_SYSTEM_NAME
    # https://stackoverflow.com/questions/28070810/cmake-generate-error-on-windows-as-it-uses-as-escape-seq
    file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOTDIR)
    # cmake_path(SET VCPKG_ROOTDIR $ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOTDIR}/scripts/buildsystems/vcpkg.cmake)
endif()

project(test-tmm-vec)

set(CMAKE_CXX_STANDARD 23)

# The current version of apt mantic packages 1.74.0 does not compile
find_package(Boost 1.83.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(test-tmm-vec test_tmm_vec.cpp
        ../../src/optics/tmm_vec.cpp
        ../../src/optics/tmm.cpp
        ../../src/tools/Approx.cpp
        ../../src/optics/FixedMatrix.cpp
        ../../src/tools/utils.cpp
)

target_include_directories(test-tmm-vec PRIVATE ${CMAKE_SOURCE_DIR}/../../src/optics ${CMAKE_SOURCE_DIR}/../../src/tools)
