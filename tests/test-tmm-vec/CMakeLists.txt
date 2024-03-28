cmake_minimum_required(VERSION 3.22)

# Using this way, we do not need to set
# -DCMAKE_TOOLCHAIN_FILE="D:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\vcpkg\scripts\buildsystems\vcpkg.cmake"
# in a specific profile for WIN32 and use a default profile for others in Settings -> Build, Execution, Deployment ->
# CMake -> CMake options.
# Run $env:VCPKG_ROOT in Developer PowerShell for VS 2022 should output:
# D:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\vcpkg
# After CLion is properly set up, we do not need to manually add a system environment variable VCPKG_ROOT.
# After configuration, remember to clear CMakeCache.txt, CMakeFiles, etc. before reloading CMake projects.
# if (CMAKE_SYSTEM_NAME STREQUAL "Windows")  # == CMAKE_HOST_SYSTEM_NAME
if (CMAKE_HOST_WIN32)  # WIN32
    # https://stackoverflow.com/questions/28070810/cmake-generate-error-on-windows-as-it-uses-as-escape-seq
    file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOTDIR)
    # cmake_path(SET VCPKG_ROOTDIR $ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOTDIR}/scripts/buildsystems/vcpkg.cmake)
    # set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/vcpkg/scripts/buildsystems/vcpkg.cmake")
    # message(CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})  # for debug use
endif()

project(test-tmm-vec)

set(CMAKE_CXX_STANDARD 23)
# If enable ASAN for MSVC, please add
# D:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.38.33130\bin\Hostx64\x64
# to path. See https://stackoverflow.com/questions/66531482/application-crashes-when-using-address-sanitizer-with-msvc.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

# apt mantic packages 1.74.0 does not compile
find_package(Boost 1.84.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(test-tmm-vec test_tmm_vec.cpp
        ../../src/optics/tmm_vec.cpp
        ../../src/optics/tmm.cpp
        ../../src/optics/FixedMatrix.cpp  # Unfortunately, this file is not used but coupled with this project.
        ../../src/utils/Approx.cpp
        ../../src/utils/Math.cpp
        ../../src/utils/Range.cpp
)

# target_include_directories(test-tmm-vec PRIVATE ${CMAKE_SOURCE_DIR}/../../src/optics ${CMAKE_SOURCE_DIR}/../../src/tools)
