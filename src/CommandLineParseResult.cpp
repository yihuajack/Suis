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
    parser.process(app);
    CommandLineParseResult result;
    result.profileDir = parser.value(profileOption).toStdString();
}
