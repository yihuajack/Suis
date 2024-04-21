//
// Created by Yihua Liu on 2024/4/8.
//

#ifndef SUISAPP_DBMODEL_H
#define SUISAPP_DBMODEL_H

#include <filesystem>
#include <QAbstractListModel>

#include "OpticMaterial.h"

class DbModel : public QAbstractListModel {
    Q_OBJECT

private:
    QMap<QString, OpticMaterial> m_list;
    std::filesystem::path m_db_imported_path;

public:
    enum Roles {
        MatNameRole = Qt::UserRole
    };

    [[nodiscard]] int rowCount(const QModelIndex &) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    void setDbImportedPath(const QString &path);
};


#endif  // SUISAPP_DBMODEL_H
