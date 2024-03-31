target_sources(SuisApp PRIVATE
        # headers
        material/OpticMaterial.h
        material/MaterialSystem.h
        optics/FixedMatrix.h
        optics/tmm.h
        utils/Approx.h
        utils/Fs.h
        utils/Log.h
        utils/Math.h
        utils/Range.h
        Application.h
        CommandLineParseResult.h
        Global.h
        Parameter.h
        Preferences.h
        Process.h
        Profile.h
        ProfilePrivate.h
        SettingsStorage.h
        # sources
        material/OpticMaterial.cpp
        material/MaterialSystem.cpp
        optics/FixedMatrix.cpp
        optics/tmm.cpp
        optics/tmm_vec.cpp
        utils/Approx.cpp
        utils/Fs.cpp
        utils/Log.cpp
        utils/Math.cpp
        utils/Range.cpp
        Application.cpp
        CommandLineParseResult.cpp
        Parameter.cpp
        Preferences.cpp
        Process.cpp
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
