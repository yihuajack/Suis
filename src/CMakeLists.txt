target_sources(SuisApp PRIVATE
        # headers
        material/MaterialDbModel.h
        material/OpticMaterial.h
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
        material/MaterialDbModel.cpp
        material/OpticMaterial.cpp
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

include(FetchContent)
FetchContent_Declare(
        QXlsx
        GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
        GIT_TAG        v1.4.8
        SOURCE_SUBDIR  QXlsx
)
FetchContent_MakeAvailable(QXlsx)

target_link_libraries(SuisApp PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        QXlsx
)

install(TARGETS SuisApp
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
