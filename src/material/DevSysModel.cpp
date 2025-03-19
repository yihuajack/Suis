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

QList<int> DevSysModel::devId() const {
    return m_devIds;
}

QStringList DevSysModel::devList() const {
    return m_devList;
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

// Db to be added in this way cannot be modified by any QML functions in ElectricalParsetPage.qml
void DevSysModel::addDeviceFromDb() {
    const QObject *sqlTreeModel = qmlEngine(this)->singletonInstance<QObject*>("content", "SqlTreeModel");
    if (not sqlTreeModel) {
        qWarning("SqlTreeModel singleton not found.");
        return;
    }
    const QVariant ids = sqlTreeModel->property("devId");
    if (const QVariant data = sqlTreeModel->property("devList"); ids.canConvert<QList<int>>() and data.canConvert<QStringList>()) {
        int id;
        QList<QVariant> devId = ids.toList();
        QStringList devList = data.toStringList();
#ifdef __cpp_lib_ranges_zip
        for (const auto [idVar, devPath] : std::views::zip(devId, devList)) {
#else
        for (qsizetype i = 0; i < devId.size(); i++) {
            if (devId.size() not_eq devList.size()) {
                qWarning("IDs size differ from data size.");
                return;
            }
            const QVariant& idVar = devId.at(i);
            const QString& devPath = devList.at(i);
#endif
            if (not idVar.canConvert<int>()) {
                qWarning("IDs cannot be converted to QList<int>.");
                return;
            }
            id = idVar.toInt();
            beginInsertRows(QModelIndex(), static_cast<int>(m_list.size()), static_cast<int>(m_list.size()));
            DeviceModel *model = new DeviceModel();
            model->setId(id);
            model->readDfDev(devPath);
            m_list.emplace_back(model);
            m_devIds.emplace_back(id);
            m_devList.emplace_back(devPath);
        }
        endInsertRows();
    } else {
        qWarning("IDs cannot be converted to QList<int> or data cannot be converted to QStringList.");
    }
}
