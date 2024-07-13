//
// Created by Yihua Liu on 2024/6/11.
//

#include <boost/property_tree/ini_parser.hpp>
#include <QFileInfo>

#include "ParameterSystem.h"

ParameterSystem::ParameterSystem(const QMap<QString, QString>& par_map, const QString& root_path) {
    for (const std::pair<QString, QString>& par_pair : par_map.asKeyValueRange()) {
        const QString& par_key = par_pair.first;
        if (par_key not_eq "calculables") {
            QString par_path = par_pair.second;
            par_path.replace("SOLCORE_ROOT", root_path);
            // Warning: Although backslash is a special character in INI files,  most Windows applications don't escape
            // backslashes (\) in file paths. QSettings always treats backslash as a special character and provides no
            // API for reading or writing such entries.
            // You have to replace all backslashes in your solcore config file on Windows!
            if (not QFileInfo::exists(par_path)) {
                qWarning() << "File not found: " << par_path;
            } else {
                boost::property_tree::ptree mat_par_ptree;
                try {
                    boost::property_tree::ini_parser::read_ini(par_path.toStdString(), mat_par_ptree);
                    for (const std::pair<const std::string, boost::property_tree::basic_ptree<std::string, std::string>>& section : mat_par_ptree) {
                        const QString group = QString::fromStdString(section.first);
                        par_settings.beginGroup(group);
                        for (const std::pair<const std::string, boost::property_tree::basic_ptree<std::string, std::string>>& elem : section.second) {
                            par_settings.setValue(QString::fromStdString(elem.first), QString::fromStdString(elem.second.data()));
                        }
                        par_settings.endGroup();
                    }
                } catch (const boost::property_tree::ini_parser_error& e) {
                    qWarning() << "Error reading INI file << " << par_path << ": " << e.what();
                }
            }
        }
    }
}

bool ParameterSystem::isComposition(const QString &mat_name, const QString& key) const {
    return par_settings.contains(mat_name + "/" + key);
}
