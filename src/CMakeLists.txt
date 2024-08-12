target_sources(SuisApp PRIVATE
        # Do not add sources of content qml module here
        # core headers
        core/BuildDevice.tpp
        core/Device.h
        core/DistFun.h
        core/GetVarSub.h
        core/ParameterClass.h
        # material headers
        material/DbSysModel.h
        material/IniConfigParser.h
        material/MaterialDbModel.h
        material/OpticMaterial.h
        material/ParameterSystem.h
        # material sources
        material/DbSysModel.cpp
        material/IniConfigParser.cpp
        material/MaterialDbModel.cpp
        material/OpticMaterial.cpp
        material/ParameterSystem.cpp
        # optics headers
        optics/FixedMatrix.h
        optics/OpticStack.h
        optics/tmm.h
        optics/TransferMatrix.h
        # optics sources
        optics/FixedMatrix.cpp
        optics/OpticStack.cpp
        optics/tmm.cpp
        optics/tmm_vec.cpp
        # sql headers
        sql/ConnectionWidget.h
        sql/ConnectionDialog.h
        sql/SqlBrowser.h
        # sql sources
        sql/ConnectionWidget.cpp
        sql/ConnectionDialog.cpp
        sql/SqlBrowser.cpp
        # sql uis
        sql/ConnectionDialog.ui
        sql/SqlBrowser.ui
        #utils headers
        utils/Approx.h
        utils/Fs.h
        utils/Log.h
        utils/Math.h
        utils/Range.h
        # utils sources
        utils/Approx.cpp
        utils/Fs.cpp
        utils/Log.cpp
        utils/Math.cpp
        utils/Range.cpp
        # top headers
        Application.h
        CommandLineParseResult.h
        Global.h
        Parameter.h
        Preferences.h
        Profile.h
        ProfilePrivate.h
        SettingsStorage.h
        # top sources
        Application.cpp
        CommandLineParseResult.cpp
        Parameter.cpp
        Preferences.cpp
        Profile.cpp
        ProfilePrivate.cpp
        SettingsStorage.cpp
        main.cpp
)
