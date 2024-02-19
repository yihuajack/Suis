cmake_minimum_required(VERSION 3.27)

option(LINK_INSIGHT "Link Qt Insight Tracker library" ON)
# If code.qt.io website server fails, turn off the following option to suppress the errors:
# [cmake] Building designer components.
# [cmake] [1/9] Creating directories for 'ds-populate'
# [cmake] [1/9] Performing download step (git clone) for 'ds-populate'
# [cmake] Cloning into 'ds-src'...
# [cmake] fatal: unable to access 'https://code.qt.io/qt-labs/qtquickdesigner-components.git/': Failed to connect to code.qt.io port 443 after 3797 ms: Couldn't connect to server
# ...
# [cmake] -- Had to git clone more than once: 3 times.
# [cmake] CMake Error at ds-subbuild/ds-populate-prefix/tmp/ds-populate-gitclone.cmake:39 (message):
# [cmake]   Failed to clone repository:
# [cmake]   'https://code.qt.io/qt-labs/qtquickdesigner-components.git'
# This corresponds to the Details (Define Project Details) step in creating applications by Qt Creator
# Uncheck "Creates a project that you can open in Qt Design Studio"
# from content/CMakeLists.txt:5 (qt6_add_qml_module)
option(BUILD_QDS_COMPONENTS "Build design studio components" ON)

project(OpenSCSimApp
    HOMEPAGE_URL "https://github.com/yihuajack/OpenSCSim"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
# Visual Studio toolchain: -DCMAKE_PREFIX_PATH=D:\Qt\6.6.2\msvc2019_64\lib\cmake
# MinGW toolchain: -DCMAKE_PREFIX_PATH=D:\Qt\6.6.2\mingw_64\lib\cmake
# Visual Studio ARM64 toolchain: -DCMAKE_PREFIX_PATH=D:\Qt\6.6.2\msvc2019_arm64\lib\cmake
# Alternatively use --toolchain instead of CMAKE_TOOLCHAIN_FILE
# set(CMAKE_PREFIX_PATH "D:\\Qt\\6.6.2\\msvc2019_64\\lib\\cmake")
# After updating MSVC, remember to manually update the path to cl.exe in
# build-OpenSCSim-Desktop_Qt_6_6_2_MSVC2019_64bit-Debug\vcpkg-dependencies\toolchain.cmake
# where CMAKE_C_COMPILER, CMAKE_CXX_COMPILER, and VCPKG_TARGET_TRIPLET are set.

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Qml Quick)
set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)
find_package(Boost 1.83.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.6)
    qt_standard_project_setup(REQUIRES 6.6)
endif()

qt_add_executable(OpenSCSimApp)

target_sources(OpenSCSimApp PRIVATE
    #headers
    src/optics/FixedMatrix.h
    src/optics/tmm.h
    src/tools/Approx.h
    src/tools/utils.h
    src/backend_init.h
    #sources
    src/optics/FixedMatrix.cpp
    src/optics/tmm.cpp
    src/optics/tmm_vec.cpp
    src/tools/Approx.cpp
    src/tools/utils.cpp
    src/backend_init.cpp
    src/main.cpp
)

qt_add_resources(OpenSCSimApp "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

target_link_libraries(OpenSCSimApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

if (BUILD_QDS_COMPONENTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/qmlcomponents)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/qmlmodules)

if (LINK_INSIGHT)
    include(${CMAKE_CURRENT_SOURCE_DIR}/insight)
endif ()

include(GNUInstallDirs)
install(TARGETS OpenSCSimApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# make IDEs aware of the QML import path
set(QML_IMPORT_PATH ${PROJECT_BINARY_DIR}/qml CACHE PATH
        "Path to the custom QML components defined by the project")

# add_subdirectory(src)
