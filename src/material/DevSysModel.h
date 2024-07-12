//
// Created by Yihua Liu on 2024-7-7.
//

#ifndef SUISAPP_DEVSYSMODEL_H
#define SUISAPP_DEVSYSMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

#include "DeviceModel.h"

// Must be public inheritance! Otherwise, qqmlprivate.h Error C2243
// 'conversion type' conversion from 'T *' to 'QObject *' exists, but is inaccessible
class DevSysModel : public QAbstractListModel {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT

public:
    enum DevSysRoles {
        NameRole = Qt::UserRole + 1,
        DeviceRole
    };

    explicit DevSysModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void addDevice();
    Q_INVOKABLE void removeDevice(const int &row);

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    QList<DeviceModel *> m_list;
};


#endif  // SUISAPP_DEVSYSMODEL_H
