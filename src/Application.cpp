//
// Created by Yihua on 2024/2/23.
//

#include <filesystem>
#include "Application.h"

using namespace std::string_literals;

namespace {
    const std::filesystem::path DEFAULT_PORTABLE_MODE_PROFILE_DIR {u"profile"s};
}

Application::Application(int &argc, char **argv) : QGuiApplication(argc, argv) {
    const std::filesystem::path portableProfilePath = QCoreApplication::applicationDirPath().toStdString() / DEFAULT_PORTABLE_MODE_PROFILE_DIR;
}
