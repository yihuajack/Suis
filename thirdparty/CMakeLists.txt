option(WITH_ORACLE "SOCI Oracle Backend" ON)
# set(SOCI_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/soci)
# https://github.com/SOCI/soci/issues/1159
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(soci)
