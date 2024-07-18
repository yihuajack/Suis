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
    // Use properties for wl & RAT because they are not expected to be accessed by model.R/A/T but device.R/A/T!
    Q_PROPERTY(QList<double> wavelength READ wavelength)
    Q_PROPERTY(QList<double> R READ readR)
    Q_PROPERTY(QList<double> A READ readA)
    Q_PROPERTY(QList<double> T READ readT)

public:
    explicit DeviceModel(QObject *parent = nullptr);

    // Refer to qtdeclarative/src/labs/models/ qqmltablemodel_p.h and qqmltablemodel.cpp
    // qtdeclarative/src/quick/items/ qquicktableview_p.h and qquicktableview.cpp
    // qtdeclarative/tests/manual/tableview/abstracttablemodel/main.cpp
    // qtbase/src/widgets/doc/snippets/common-table-model/ model.h and model.cpp
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    [[nodiscard]] QString name() const;
    [[nodiscard]] QList<double> wavelength() const;
    [[nodiscard]] QList<double> readR() const;
    [[nodiscard]] QList<double> readA() const;
    [[nodiscard]] QList<double> readT() const;

    // We do not allow editing headers
    Q_INVOKABLE [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    // Q_INVOKABLE void copyToClipboard(const QModelIndexList &indexes) const;
    // Q_INVOKABLE bool pasteFromClipboard(const QModelIndex &targetIndex);

    Q_INVOKABLE bool readDfDev(const QString &db_path);
    Q_INVOKABLE void calcRAT();

private:
    ParameterClass<QList, double, QString, QVariant> par;  // by column
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
