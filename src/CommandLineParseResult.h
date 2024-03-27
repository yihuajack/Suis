//
// Created by Yihua Liu on 2024/3/27.
//

#ifndef SUISAPP_COMMANDLINEPARSERESULT_H
#define SUISAPP_COMMANDLINEPARSERESULT_H

#include <filesystem>
// Just use QCommandLineParser instead of boost::property_tree. Boost libraries lack maintenance and documentation.
#include <QCommandLineParser>

struct CommandLineParseResult {
    std::filesystem::path profileDir;
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, const QCoreApplication &app);

#endif  // SUISAPP_COMMANDLINEPARSERESULT_H
