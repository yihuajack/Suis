target_sources(SuisApp PRIVATE
        # headers
        optics/FixedMatrix.h
        optics/tmm.h
        utils/Approx.h
        utils/Fs.h
        utils/Log.h
        utils/Math.h
        utils/Range.h
        backend_init.h
        Application.h
        CommandLineParseResult.h
        Config.h
        Global.h
        Preferences.h
        Profile.h
        ProfilePrivate.h
        SettingsStorage.h
        # sources
        optics/FixedMatrix.cpp
        optics/tmm.cpp
        optics/tmm_vec.cpp
        utils/Approx.cpp
        utils/Fs.cpp
        utils/Log.cpp
        utils/Math.cpp
        utils/Range.cpp
        backend_init.cpp
        Application.cpp
        CommandLineParseResult.cpp
        Config.cpp
        Preferences.cpp
        Profile.cpp
        ProfilePrivate.cpp
        SettingsStorage.cpp
        main.cpp
)

target_link_libraries(SuisApp PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)

install(TARGETS SuisApp
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
