target_sources(OpenSCSim PRIVATE
        #headers
        optics/FixedMatrix.h
        optics/tmm.h
        tools/Approx.h
        tools/utils.h
        backend_init.h
        Config.h
        #sources
        optics/FixedMatrix.cpp
        optics/tmm.cpp
        optics/tmm_vec.cpp
        tools/Approx.cpp
        tools/utils.cpp
        backend_init.cpp
        Config.cpp
        main.cpp
)

qt_add_resources(OpenSCSim "configuration"
        PREFIX "/"
        FILES
        qtquickcontrols2.conf
)

target_link_libraries(OpenSCSim PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)

install(TARGETS OpenSCSim
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
