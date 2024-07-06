//
// Created by Yihua Liu on 2024-06-17.
//

#include "DbSysModel.h"

DbSysModel::DbSysModel(QObject *parent) : QAbstractListModel(parent) {
    MaterialDbModel *solcore_model = new MaterialDbModel(parent, "Solcore");
    MaterialDbModel *df_model = new MaterialDbModel(parent, "Df");
    addModel(solcore_model);
    addModel(df_model);
}

int DbSysModel::rowCount(const QModelIndex &parent = QModelIndex()) const {
    Q_UNUSED(parent);
    return m_db.count();
}

QVariant DbSysModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 or index.row() >= m_db.count()) {
        return {};
    }
    const MaterialDbModel *db = m_db[index.row()];
    switch (role) {
        case NameRole:
            return db->name();
        case CheckedRole:
            return db->checked();
        case PathRole:
            return db->path();
        case ProgressRole:
            return db->progress();
        case ModelRole:
            return QVariant::fromValue(db);
        default:
            return {};
    }
}

bool DbSysModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.row() < 0 or index.row() >= m_db.count()) {
        return false;
    }
    MaterialDbModel *db = m_db[index.row()];
    switch (role) {
        case ProgressRole:
            db->setProgress(value.toDouble());
            emit dataChanged(index, index, {role});
            return true;
        case CheckedRole:
            db->setChecked(value.toBool());
            emit dataChanged(index, index, {role});
            return true;
        case PathRole:
            db->setPath(value.toString());
            emit dataChanged(index, index, {role});
            return true;
        default:
            return false;
    }
}

void DbSysModel::addModel(MaterialDbModel *db_model) {
    connect(db_model, &MaterialDbModel::progressChanged, this, &DbSysModel::onProgressChanged);

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_db.append(db_model);
    endInsertRows();
}

QHash<int, QByteArray> DbSysModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[CheckedRole] = "checked";
    roles[PathRole] = "path";
    roles[ProgressRole] = "progress";
    roles[ModelRole] = "db_model";
    return roles;
}

void DbSysModel::onProgressChanged() {
    MaterialDbModel *db_model = qobject_cast<MaterialDbModel *>(sender());
    if (db_model) {
        int row = m_db.indexOf(db_model);
        if (row >= 0) {
            QModelIndex index = createIndex(row, 0);
            emit dataChanged(index, index, {ProgressRole});
        }
    }
}
