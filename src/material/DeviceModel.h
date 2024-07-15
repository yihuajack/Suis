//
// Created by Yihua Liu on 2024-7-1.
//

#ifndef SUISAPP_DEVICEMODEL_H
#define SUISAPP_DEVICEMODEL_H

#include <QAbstractTableModel>
#include <QQmlEngine>

#include "core/ParameterClass.h"
#include "optics/OpticStack.h"

class DeviceModel : public QAbstractTableModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString name READ name)

public:
    enum ModelRoles {
        NameRole = Qt::UserRole + 1,
        WlRole,
        RRole,
        ARole,
        TRole
    };

    explicit DeviceModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QString name() const;

    Q_INVOKABLE bool readDfDev(const QString &db_path);
    Q_INVOKABLE void calcRAT();

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    ParameterClass par;  // by column
    QString m_name;

    // for calcRAT()
    QList<QString> opt_material;
    QList<double> opt_d;
    QList<double> wavelengths;
    QList<double> R;
    QList<double> A;
    QList<double> T;

    bool import_properties(const QList<QStringList> &data, const std::map<QString, qsizetype> &properties);
};

#endif  // SUISAPP_DEVICEMODEL_H
