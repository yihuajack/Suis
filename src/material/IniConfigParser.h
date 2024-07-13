//
// Created by Yihua Liu on 2024-06-09.
//

#ifndef SUISAPP_INICONFIGPARSER_H
#define SUISAPP_INICONFIGPARSER_H

#include <QSettings>
#include <QString>

class IniConfigParser {
public:
    explicit IniConfigParser(const QString& ini_fn);
    QMap<QString, QString> loadGroup(const QString& group_name);
private:
    QSettings ini_settings;
};


#endif  // SUISAPP_INICONFIGPARSER_H
