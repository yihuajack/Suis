//
// Created by Yihua Liu on 2024/4/8.
//

#include "CustomDbModel.h"

CustomDbModel::CustomDbModel(const QString &db_name) {

}

bool CustomDbModel::setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) {
    if (!value.canConvert<QString>() && role != Qt::EditRole) {
        return false;
    }

    auto it = m_list.begin() + index.row();
    QString charactersUnformatted = value.toString();
    QStringList characters = charactersUnformatted.split(", ");

    emit dataChanged(index, index, {role});

    return true;
}
