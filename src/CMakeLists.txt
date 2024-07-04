target_sources(SuisApp PRIVATE
        # Do not add sources of content qml module here
        # headers
        material/IniConfigParser.h
        material/MaterialDbModel.h
        material/OpticMaterial.h
        material/ParameterSystem.h
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
        Profile.h
        ProfilePrivate.h
        SettingsStorage.h
        # sources
        material/IniConfigParser.cpp
        material/MaterialDbModel.cpp
        material/OpticMaterial.cpp
        material/ParameterSystem.cpp
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
        Profile.cpp
        ProfilePrivate.cpp
        SettingsStorage.cpp
        main.cpp
)
