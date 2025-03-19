//
// Created by Yihua Liu on 2024-7-1.
//

#ifndef SUISAPP_DEVICEMODEL_H
#define SUISAPP_DEVICEMODEL_H

#include <QAbstractTableModel>

#include "core/ParameterClass.h"

class DeviceModel : public QAbstractTableModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString path READ path CONSTANT)
    Q_PROPERTY(bool isImported READ isImported NOTIFY importChanged)
    // Use properties for wl & RAT because they are not expected to be accessed by model.R/A/T but device.R/A/T!
    Q_PROPERTY(QList<double> wavelength READ wavelength CONSTANT)
    Q_PROPERTY(QList<double> R READ readR CONSTANT)
    Q_PROPERTY(QList<double> A READ readA CONSTANT)
    Q_PROPERTY(QList<double> T READ readT CONSTANT)
    // QQmlExpression: Expression qrc:/qt/qml/content/BandDiagramDialog.qml: depends on non-NOTIFYable properties
    Q_PROPERTY(qsizetype col_size READ readColSize CONSTANT);
    Q_PROPERTY(QList<double> d READ readD CONSTANT)
    Q_PROPERTY(QList<double> CBM READ readCBM CONSTANT)
    Q_PROPERTY(QList<double> VBM READ readVBM CONSTANT)

signals:
    void idChanged();
    void importChanged();

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
    [[nodiscard]] int id() const;
    void setId(int id);
    [[nodiscard]] QString path() const;
    [[nodiscard]] bool isImported() const;
    [[nodiscard]] QList<double> wavelength() const;
    [[nodiscard]] QList<double> readR() const;
    [[nodiscard]] QList<double> readA() const;
    [[nodiscard]] QList<double> readT() const;
    [[nodiscard]] qsizetype readColSize() const;
    [[nodiscard]] QList<double> readD() const;
    [[nodiscard]] QList<double> readCBM() const;
    [[nodiscard]] QList<double> readVBM() const;

    // We do not allow editing headers
    Q_INVOKABLE [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    // Q_INVOKABLE void copyToClipboard(const QModelIndexList &indexes) const;
    // Q_INVOKABLE bool pasteFromClipboard(const QModelIndex &targetIndex);

    Q_INVOKABLE void readDfDev(const QString &db_path);
    Q_INVOKABLE void calcRAT();

private:
    std::unique_ptr<ParameterClass<QList, double, QString>> par;  // by column
    QString m_name;
    int m_id = 0;
    QString m_path;
    bool imported = false;  // readDfDev does not return a bool because it may be called from the C++ side

    // for calcRAT()
    QList<QString> opt_material;
    QList<double> opt_d;
    QList<double> wavelengths;
    QList<double> R;
    QList<double> A;
    QList<double> T;
};

#endif  // SUISAPP_DEVICEMODEL_H
