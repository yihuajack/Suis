//
// Created by Yihua Liu on 2024/4/8.
//

#include "DbModel.h"

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

void DbModel::setDbImportedPath(const QString &path) {
    m_db_imported_path = path.toStdString();
}
