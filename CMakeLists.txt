cmake_minimum_required(VERSION 3.22)

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
option(BUILD_TESTS "Enable building tests" OFF)

project(SuisApp
        VERSION 1.0.0
        DESCRIPTION "Open-source Solar Cell Simulator"
        HOMEPAGE_URL "https://github.com/yihuajack/Suis"
        LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# CMake Arguments:
# -DCMAKE_TOOLCHAIN_FILE="D:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\vcpkg\scripts\buildsystems\vcpkg.cmake"
# Visual Studio toolchain: -DCMAKE_PREFIX_PATH=D:\Qt\<qtver>\msvc2022_64\lib\cmake
# MinGW toolchain: -DCMAKE_PREFIX_PATH=D:\Qt\<qtver>\mingw_64\lib\cmake
# MSVC 2022 ARM64: msvc2022_arm64, LLVM-MinGW: llvm-mingw_64
# Alternatively use --toolchain instead of CMAKE_TOOLCHAIN_FILE
# set(CMAKE_PREFIX_PATH "D:\\Qt\\<qtver>\\msvc2022_64\\lib\\cmake")
# Could NOT find WrapVulkanHeaders (missing: Vulkan_INCLUDE_DIR)
# See https://bugreports.qt.io/browse/QTBUG-92587
# https://gitlab.kitware.com/cmake/cmake/-/merge_requests/3027
# Every time add new qt packages here on Windows CLion remember to run `windeployqt --qmldir qml SuisApp.exe` under
# `cmake-build-debug-visual-studio` in Qt shell
# (see https://blog.csdn.net/yihuajack/article/details/136925071) or refer to
# https://stackoverflow.com/questions/59187030/why-can-clion-correctly-build-and-link-qt-but-not-run-my-executable.
find_package(Qt6 6.8 REQUIRED COMPONENTS Charts Core Gui Qml Quick Sql)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# use ppa:mhier/libboost-latest and install libboost1.83-all-dev in jammy ubuntu
find_package(Boost 1.83.0 REQUIRED)
# Check default CMAKE_FIND_LIBRARY_SUFFIXES in Modules/CMakeGenericSystem.cmake ("lib"/[".so" ".a"]) and
# Modules/Platform/Windows.cmake (["" "lib"]/["dll.lib" ".lib" ".a"]) for MSVC
# SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
# SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a")
# find_package(Octave REQUIRED COMPONENTS Development Interpreter)
# add_definitions(-DCMAKE_EXPECTED_Octave_VERSION=${Octave_VERSION})

include_directories(${Boost_INCLUDE_DIRS})
# https://stackoverflow.com/questions/74854202/qmltyperegistration-include-path-does-not-acknowledge-subdirectories
include_directories(src)  # for content/CMakeLists.txt qt6_add_qml_module SOURCES
include_directories(src/material)
include_directories(src/sql) # https://forum.qt.io/topic/156560/header-not-found-in-appname-_qmltyperegistrations-cpp

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.8)
    qt_standard_project_setup(REQUIRES 6.8)
endif()

qt_add_executable(SuisApp WIN32 MACOSX_BUNDLE)

qt_add_resources(SuisApp "configuration"
        PREFIX "/"
        FILES
        qtquickcontrols2.conf
)

add_subdirectory(src)
include_directories(${ORACLE_INCLUDE_DIR})

include(FetchContent)
FetchContent_Declare(
        QXlsx
        GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
        GIT_TAG        v1.5.0
        SOURCE_SUBDIR  QXlsx
)
FetchContent_MakeAvailable(QXlsx)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(SuisApp PROPERTIES
        # MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appQuickTest
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(SuisApp PRIVATE
        Qt6::Charts
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Sql
        QXlsx::QXlsx
)

include(GNUInstallDirs)
install(TARGETS SuisApp
        BUNDLE DESTINATION .
        # https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (BUILD_QDS_COMPONENTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/qmlcomponents)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/qmlmodules)

if (LINK_INSIGHT)
    include(${CMAKE_CURRENT_SOURCE_DIR}/insight)
endif ()

# make IDEs aware of the QML import path
set(QML_IMPORT_PATH ${PROJECT_BINARY_DIR}/qml CACHE PATH
        "Path to the custom QML components defined by the project")

qt_generate_deploy_qml_app_script(
        TARGET SuisApp
        OUTPUT_SCRIPT deploy_script
        MACOS_BUNDLE_POST_BUILD
        NO_UNSUPPORTED_PLATFORM_ERROR
        DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)

install(SCRIPT ${deploy_script})
