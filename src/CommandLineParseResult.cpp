//
// Created by Yihua Liu on 2024/3/27.
//

#include "CommandLineParseResult.h"

using namespace Qt::Literals::StringLiterals;

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, const QCoreApplication &app) {
    parser.setApplicationDescription(u"Suis - Solar Cell Simulator"_s);
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption profileOption("profile", "The path to profile directory");
    parser.addOption(profileOption);
    const QCommandLineOption configNameOption("config", "Store configuration files in <dir>");
    parser.addOption(configNameOption);
    parser.process(app);
    CommandLineParseResult result;
    result.profileDir = std::filesystem::absolute(parser.value(profileOption).toStdString());
    result.configName = parser.value(configNameOption).toStdU16String();
    return result;
}
