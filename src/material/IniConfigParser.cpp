//
// Created by Yihua Liu on 2024-06-09.
//

#include "IniConfigParser.h"

IniConfigParser::IniConfigParser(const QString &ini_fn) : ini_settings(ini_fn, QSettings::IniFormat) {}

QVariantMap IniConfigParser::loadGroup(const QString& group_name) {
    QVariantMap group_map;
    ini_settings.beginGroup(group_name);
    QStringList keys = ini_settings.childKeys();
    for (const QString& key : keys) {
        group_map.insert(key, ini_settings.value(key));
    }
    ini_settings.endGroup();
    return group_map;
}
