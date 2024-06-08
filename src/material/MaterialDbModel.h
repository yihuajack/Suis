//
// Created by Yihua Liu on 2024/6/3
//

#ifndef SUISAPP_MATERIALDBMODEL_H
#define SUISAPP_MATERIALDBMODEL_H

#include <QObject>
#include <QVariantMap>

#include "OpticMaterial.h"

class MaterialDbModel : public QObject {
    Q_OBJECT
public:
    explicit MaterialDbModel(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap readSolcoreDb(const QString& folderPath);
    Q_INVOKABLE QVariantMap readDfDb(const QString& filePath);

private:
    QMap<QString, OpticMaterial> m_list;
};

#endif  // SUISAPP_MATERIALDBMODEL_H
