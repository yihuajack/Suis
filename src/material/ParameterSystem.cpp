//
// Created by Yihua Liu on 2024/6/11.
//

#include <boost/property_tree/ini_parser.hpp>
#include <QFileInfo>

#include "IniConfigParser.h"
#include "ParameterSystem.h"

ParameterSystem::ParameterSystem(const QMap<QString, QString>& par_map, const QString& root_path) {
    QString calc_par_path = par_map.value("calculables");
    calc_par_path.replace("SOLCORE_ROOT", root_path);
    for (const auto& [par_key, mat_par_path] : par_map.asKeyValueRange()) {
        if (par_key not_eq "calculables") {
            if (not QFileInfo::exists(mat_par_path)) {
                qWarning() << "File not found: " << mat_par_path;
            } else {
                boost::property_tree::ptree mat_par_ptree;
                try {
                    boost::property_tree::ini_parser::read_ini(mat_par_path.toStdString(), mat_par_ptree);
                    for (const std::pair<const std::string, boost::property_tree::basic_ptree<std::string, std::string>>& section : mat_par_ptree) {
                        const QString group = QString::fromStdString(section.first);
                        par_settings.beginGroup(group);
                        for (const std::pair<const std::string, boost::property_tree::basic_ptree<std::string, std::string>>& elem : section.second) {
                            par_settings.setValue(QString::fromStdString(elem.first), QString::fromStdString(elem.second.data()));
                        }
                        par_settings.endGroup();
                    }
                } catch (const boost::property_tree::ini_parser_error& e) {
                    qWarning() << "Error reading INI file: " << e.what();
                }
            }
        }
    }
    IniConfigParser calc_par(calc_par_path);
}

bool ParameterSystem::isComposition(const QString &mat_name, const QString& key) const {
    return par_settings.contains(mat_name + "/" + key);
}
