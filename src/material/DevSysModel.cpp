//
// Created by Yihua Liu on 2024-7-7.
//

#include "DevSysModel.h"

DevSysModel::DevSysModel(QObject *parent) : QAbstractListModel(parent) {}

int DevSysModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(m_list.count());
}

QVariant DevSysModel::data(const QModelIndex &index, int role) const {
    if (not index.isValid() or index.row() >= m_list.count()) {
        qWarning("QModelIndex of DevSysModel is invalid.");
        return {};
    }
    const DeviceModel *dev = m_list[index.row()];
    switch (role) {
        case NameRole:
            return dev->name();
        case DeviceRole:
            return QVariant::fromValue(dev);
        default:
            return {};
    }
}

QHash<int, QByteArray> DevSysModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DeviceRole] = "device";
    return roles;
}

void DevSysModel::addDevice() {
    // Do not minus 1! Otherwise, for Qt 6.7.2,
    // ASSERT: "first >= 0" in file C:\Users\qt\work\qt\qtbase\src\corelib\itemmodels\qabstractitemmodel.cpp, line 2886
    // Q_ASSERT(first >= 0) in void QAbstractItemModel::beginInsertRows(const QModelIndex &parent, int first, int last)
    beginInsertRows(QModelIndex(), static_cast<int>(m_list.size()), static_cast<int>(m_list.size()));
    m_list.emplace_back(new DeviceModel());
    endInsertRows();
    // emit dataChanged(index(0), index(static_cast<int>(m_list.size() - 1)));
}

void DevSysModel::removeDevice(const int &row) {
    if (row < 0 or row >= static_cast<int>(m_list.size())) {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_list.erase(m_list.cbegin() + row);
    endRemoveRows();
    // emit dataChanged(index(0), index(static_cast<int>(m_list.size() - 1)));
}
