//
// Created by Yihua Liu on 2024/3/27.
//

#include "CommandLineParseResult.h"

using namespace Qt::Literals::StringLiterals;

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, const QCoreApplication &app) {
    parser.setApplicationDescription(u"Suis - Solar Cell Simulator"_s);
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption profileOption("profile",
                                           QCoreApplication::translate("main", "The path to profile <directory>"),
                                           QCoreApplication::translate("main", "directory"));
    parser.addOption(profileOption);
    const QCommandLineOption configNameOption("config",
                                              QCoreApplication::translate("main", "Store configuration files in <dir>"),
                                              QCoreApplication::translate("main", "dir"));
    parser.addOption(configNameOption);
    parser.process(app);
    CommandLineParseResult result;
    result.profileDir = std::filesystem::absolute(parser.value(profileOption).toStdString());
    result.configName = parser.value(configNameOption).toStdU16String();
    return result;
}
