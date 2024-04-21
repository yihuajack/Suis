//
// Created by Yihua Liu on 2024/4/8.
//

#ifndef SUISAPP_CUSTOM_DBMODEL_H
#define SUISAPP_CUSTOM_DBMODEL_H

#include <filesystem>
#include <QAbstractListModel>

#include "DbModel.h"
#include "OpticMaterial.h"

class CustomDbModel : public DbModel {
    Q_OBJECT

private:
    QMap<QString, OpticMaterial> m_list;

public:
    explicit CustomDbModel(const QString &db_name);
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};


#endif  // SUISAPP_CUSTOM_DBMODEL_H
