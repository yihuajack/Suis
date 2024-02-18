cmake_minimum_required(VERSION 3.27)

option(LINK_INSIGHT "Link Qt Insight Tracker library" ON)
option(BUILD_QDS_COMPONENTS "Build design studio components" OFF)

# Visual Studio toolchain: -DCMAKE_PREFIX_PATH=D:\Qt\6.6.2\msvc2019_64\lib\cmake
# MinGW toolchain: -DCMAKE_PREFIX_PATH=D:\Qt\6.6.2\mingw_64\lib\cmake
# Visual Studio ARM64 toolchain: -DCMAKE_PREFIX_PATH=D:\Qt\6.6.2\msvc2019_arm64\lib\cmake
project(OpenSCSimApp
    HOMEPAGE_URL "https://github.com/yihuajack/OpenSCSim"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(Boost 1.83.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.6)
    qt_standard_project_setup()
endif()

qt_add_executable(OpenSCSimApp src/main.cpp
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

add_subdirectory(src)
