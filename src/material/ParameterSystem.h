//
// Created by Yihua Liu on 2024/6/11.
//

#ifndef SUISAPP_PARAMETERSYSTEM_H
#define SUISAPP_PARAMETERSYSTEM_H

#include <QSettings>

class ParameterSystem {
public:
    ParameterSystem(const QMap<QString, QString>& par_map, const QString& root_path);
    [[nodiscard]] bool isComposition(const QString& mat_name, const QString& key) const;

private:
    QSettings par_settings;
};


#endif  // SUISAPP_PARAMETERSYSTEM_H
