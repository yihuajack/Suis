include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(scsim-base STATIC
        #headers
        optics/FixedMatrix.h
        optics/tmm.h
        tools/Approx.h
        tools/utils.h
        backend_init.h
        #sources
        optics/FixedMatrix.cpp
        optics/tmm.cpp
        optics/tmm_vec.cpp
        tools/Approx.cpp
        tools/utils.cpp
        backend_init.cpp
)

target_link_libraries(scsim-base
        PRIVATE
        Boost
)
