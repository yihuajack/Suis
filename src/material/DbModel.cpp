//
// Created by Yihua Liu on 2024/4/8.
//

#include <QDir>
#include <QFile>

#include "DbModel.h"

void DbModel::import() {

}

DbModel::DbModel(QString db_name) : db_name(std::move(db_name)) {}

int DbModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(m_list.count());
}

QHash<int, QByteArray> DbModel::roleNames() const {
    QHash<int, QByteArray> map = {
            {MatNameRole,  "name"}
    };
    return map;
}

QVariant DbModel::data(const QModelIndex &index, int role) const {
    const QMap<QString, OpticMaterial>::const_iterator it = m_list.begin() + index.row();
    if (role == MatNameRole) {
        return it.key();
    } else {
        return {};
    }
}

QString DbModel::dbPath() const {
    return m_db_path;
}

void DbModel::setDbPath(const QString &db_path) {
    if (m_db_path not_eq db_path) {
        m_db_path = db_path;
        emit dbPathChanged();
        readDb();
    }
}

void DbModel::readDb() {
    beginResetModel();

    if (m_db_name not_eq "Df") {
        QDir db_dir(m_db_path);
        if (not db_dir.exists()) {
            qWarning("Cannot find the example directory");
        }
        db_dir.setFilter(QDir::Dirs);
        if (m_db_name == "Solcore") {
            QStringList name_filters;
            name_filters << "*-Material";
            db_dir.setNameFilters(name_filters);
            QStringList subdir_list = db_dir.entryList();  // db_dir.entryList(nameFilters, filter)
            for (const QString& subdir_name: subdir_list) {
                const QString mat_name = subdir_name.split('-').front();
                OpticMaterial opt_mat(mat_name, subdir_name);
                m_list.insert(mat_name, opt_mat);
            }
        }
    } else {
        QFile db_file(m_db_path);
    }

    endResetModel();
}
