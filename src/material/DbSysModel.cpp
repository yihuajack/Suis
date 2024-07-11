//
// Created by Yihua Liu on 2024-06-17.
//

#include "DbSysModel.h"

DbSysModel::DbSysModel(QObject *parent) : QAbstractListModel(parent) {
    auto *solcore_model = new MaterialDbModel(parent, "Solcore");
    auto *df_model = new MaterialDbModel(parent, "Df");
    addModel(solcore_model);
    addModel(df_model);
}

int DbSysModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(m_db.count());
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

    // rowCount()
    beginInsertRows(QModelIndex(), static_cast<int>(m_db.size()), static_cast<int>(m_db.size()));
    m_db.append(db_model);
    endInsertRows();
    emit dataChanged(index(0), index(static_cast<int>(m_db.size() - 1)));
}

OpticMaterial<QList<double>> *DbSysModel::getMatByName(const QString &mat_name) const {
    for (const MaterialDbModel *mat_db : m_db) {
        OpticMaterial<QList<double>> *opt_mat = mat_db->getMatByName(mat_name);
        if (opt_mat) {
            return opt_mat;
        }
    }
    qWarning() << mat_name << " not found in DbSysModel";
    return nullptr;
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
    auto *db_model = qobject_cast<MaterialDbModel *>(sender());
    if (db_model) {
        int row = static_cast<int>(m_db.indexOf(db_model));
        if (row >= 0) {
            QModelIndex index = createIndex(row, 0);
            emit dataChanged(index, index, {ProgressRole});
        }
    }
}
