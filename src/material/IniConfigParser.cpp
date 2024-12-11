//
// Created by Yihua Liu on 2024-06-09.
//

#include "IniConfigParser.h"

IniConfigParser::IniConfigParser(const QString &ini_fn) : ini_settings(ini_fn, QSettings::IniFormat) {}

QMap<QString, QString> IniConfigParser::loadGroup(const QString& group_name) {
    QMap<QString, QString> group_map;
    ini_settings.beginGroup(group_name);
    for (QStringList keys = ini_settings.childKeys(); const QString& key : keys) {
        group_map.insert(key, ini_settings.value(key).toString());
    }
    ini_settings.endGroup();
    return group_map;
}
