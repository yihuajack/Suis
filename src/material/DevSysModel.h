//
// Created by Yihua Liu on 2024-7-7.
//

#ifndef SUISAPP_DEVSYSMODEL_H
#define SUISAPP_DEVSYSMODEL_H

#include <QQmlEngine>

#include "DeviceModel.h"

// Must be public inheritance! Otherwise, qqmlprivate.h Error C2243
// 'conversion type' conversion from 'T *' to 'QObject *' exists, but is inaccessible
class DevSysModel : public QAbstractListModel {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
    Q_PROPERTY(QList<int> devId READ devId NOTIFY devIdChanged)
    Q_PROPERTY(QStringList devList READ devList NOTIFY devListChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    enum DevSysRoles {
        NameRole = Qt::UserRole + 1,
        DeviceRole,
        SelectedRole
    };

    explicit DevSysModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QList<int> devId() const;
    [[nodiscard]] QStringList devList() const;
    [[nodiscard]] int selectedIndex() const;

    void setSelectedIndex(int selectedIndex);

    Q_INVOKABLE void addDevice();
    Q_INVOKABLE void removeDevice(const int &row);
    Q_INVOKABLE void addDeviceFromDb();

signals:
    void devIdChanged();
    void devListChanged();
    void selectedIndexChanged();

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    QList<DeviceModel *> m_list;

    QList<int> m_devIds;
    QStringList m_devList;
    int m_selectedIndex = -1;
};


#endif  // SUISAPP_DEVSYSMODEL_H
