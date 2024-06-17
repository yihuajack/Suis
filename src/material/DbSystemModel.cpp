//
// Created by Yihua Liu on 2024-06-17.
//

#include "DbSystemModel.h"

DbSystemModel::DbSystemModel(QObject *parent) : QAbstractListModel(parent) {}

int DbSystemModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_db.count();
}

QVariant DbSystemModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 or index.row() >= m_db.count()) {
        return {};
    }
    const MaterialDbModel &db = m_db[index.row()];
    switch (role) {
        case NameRole:
            return db.db_name;
        case CheckedRole:
            return db.checked;
        default:
            return {};
    }
}

QHash<int, QByteArray> DbSystemModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[CheckedRole] = "checked";
    return roles;
}
