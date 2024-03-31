//
// Created by Yihua Liu on 2024/2/23.
//

#include <filesystem>

#include "Application.h"
#include "Preferences.h"
#include "Profile.h"
#include "SettingsStorage.h"

using namespace std::string_literals;  // equivalent to std::literals::string_literals

namespace {
    const std::filesystem::path DEFAULT_PORTABLE_MODE_PROFILE_DIR {u"profile"s};
}

Application::Application(int &argc, char **argv) : QGuiApplication(argc, argv) {
    QCommandLineParser parser;
    m_commandLineArgs = parseCommandLine(parser, *this);
    const std::filesystem::path portableProfilePath = QCoreApplication::applicationDirPath().toStdString() / DEFAULT_PORTABLE_MODE_PROFILE_DIR;
    const bool portableModeEnabled = m_commandLineArgs.profileDir.empty() and std::filesystem::exists(portableProfilePath);
    const std::filesystem::path profileDir = portableModeEnabled ? portableProfilePath : m_commandLineArgs.profileDir;
    Profile::initInstance(profileDir, m_commandLineArgs.configName);
    SettingsStorage::initInstance();
    Preferences::initInstance();
}
