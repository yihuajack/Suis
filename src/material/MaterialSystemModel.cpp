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
    return static_cast<int>(m_list.count());
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

    const QString db_path_str = value.toString();
    const QMap<QString, DbModel*>::const_iterator it = m_list.cbegin() + index.row();
    if (role == DbImportedPathRole) {
        // Note that what the Qt documentation does not clearly explain is that
        // QDir::AllDirs means exactly to ignore regular expression name filters
        // and forces to push hidden directories.
        // QDirs does not push hidden directories unless explicitly requested by QDir::Hidden.
        // See the source code of `qdiriterator.cpp`.
        // Both of them never pushes the current directory "." and the parent directory "..".
        // Both of them do not follow symbolic links unless explicitly requested.
        // QDir::entryList -> QDirIterator::next -> QDirIteratorPrivate::advance
        // -> QDirIteratorPrivate::entryMatches -> QDirIteratorPrivate::checkAndPushDirectory,
        // QDirIteratorPrivate::matchesFilters.
        DbModel solcore_db_model(it.key());
        m_list.insert(it.key(), &solcore_db_model);
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
