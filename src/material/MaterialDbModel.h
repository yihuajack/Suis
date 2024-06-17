//
// Created by Yihua Liu on 2024/6/3.
//

#ifndef SUISAPP_MATERIALDBMODEL_H
#define SUISAPP_MATERIALDBMODEL_H

#include <QAbstractListModel>

#include "CompOpticMaterial.h"
#include "OpticMaterial.h"

class MaterialDbModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(double progress READ getProgress WRITE setProgress NOTIFY progressChanged)

public:
    enum ModelRoles {
        NameRole = Qt::UserRole + 1,
        ValueRole
    };
    QString db_name;
    bool checked;

    explicit MaterialDbModel(QObject *parent = nullptr);

    // Default arguments on virtual or override methods are prohibited
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;  // parent = QModelIndex()
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;  // role = Qt::DisplayRole

    [[nodiscard]] double getProgress() const;
    void setProgress(int progress);

    Q_INVOKABLE QVariantMap readSolcoreDb(const QString& db_path);
    Q_INVOKABLE QVariantMap readDfDb(const QString& db_path);

signals:
    void progressChanged(double progress);

private:
    // If using QObject, the values should be a pointer
    QMap<QString, CompOpticMaterial *> m_comp_list;
    QMap<QString, OpticMaterial *> m_list;

    double import_progress;
};

#endif  // SUISAPP_MATERIALDBMODEL_H
