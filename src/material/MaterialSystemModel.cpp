//
// Created by Yihua Liu on 2024/4/8.
//

#include <QDir>

#include "MaterialSystemModel.h"

MaterialSystemModel::MaterialSystemModel() {
    DbModel DfMatList;
    m_list.insert("Df", &DfMatList);
}

int MaterialSystemModel::rowCount(const QModelIndex &) const {
    return m_list.count();
}

QHash<int, QByteArray> MaterialSystemModel::roleNames() const {
    QHash<int, QByteArray> map = {
            {DbNameRole,  "db_name"},
            {DbImportedPathRole, "import_path"}
    };
    return map;
}

QVariant MaterialSystemModel::data(const QModelIndex &index, int role) const {
    const QMap<QString, DbModel*>::const_iterator it = m_list.begin() + index.row();
    switch (role) {
        case DbNameRole:
            return it.key();
        default:
            return {};
    }
}

// Default arguments on virtual or override methods are prohibited
bool MaterialSystemModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (not value.canConvert<QString>() and role not_eq Qt::EditRole) {
        return false;
    }

    QString db_path_str = value.toString();
    const QMap<QString, DbModel*>::const_iterator it = m_list.cbegin() + index.row();
    if (role == DbImportedPathRole) {
        m_list[it.key()]->setDbImportedPath(value.toString());
        QDir db_dir(db_path_str);
        if (it.key() == "Suis") {
            if (not db_dir.exists()) {
                qWarning("Cannot find the example directory");
            }
        }
    }

    emit dataChanged(index, index, {role});

    return true;
}

void MaterialSystemModel::addSpecies(const QString &db_name) {
    int end_m_list = static_cast<int>(m_list.size()) - 1;
    beginInsertRows(QModelIndex(),  end_m_list, end_m_list);
    CustomDbModel custom_db(db_name);
    m_list.insert(db_name, &custom_db);
    endInsertRows();
    emit dataChanged(index(0), index(end_m_list));
}

void MaterialSystemModel::deleteSpecies(const QString &species, const int &rowIndex) {
    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    m_list.remove(species);
    endRemoveRows();
    emit dataChanged(index(0), index(static_cast<int>(m_list.size()) - 1));
}
