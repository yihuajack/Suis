//
// Created by Yihua Liu on 2024-06-17.
//

#ifndef SUISAPP_DBSYSTEMMODEL_H
#define SUISAPP_DBSYSTEMMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

#include "MaterialDbModel.h"

class DbSystemModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum DbSysRoles {
        NameRole = Qt::UserRole + 1,
        CheckedRole
    };

    explicit DbSystemModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    QList<MaterialDbModel> m_db;
};

#endif  // SUISAPP_DBSYSTEMMODEL_H
