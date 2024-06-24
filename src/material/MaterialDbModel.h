//
// Created by Yihua Liu on 2024/6/3.
//

#ifndef SUISAPP_MATERIALDBMODEL_H
#define SUISAPP_MATERIALDBMODEL_H

#include <QAbstractListModel>

#include "OpticMaterial.h"

class MaterialDbModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(double progress READ getProgress WRITE setProgress NOTIFY progressChanged)
    // Sources include QXlsx library headers so is hard to register by QML_ELEMENT

public:
    enum ModelRoles {
        NameRole = Qt::UserRole + 1,
        NWlRole
    };

    explicit MaterialDbModel(QObject *parent = nullptr);

    // Default arguments on virtual or override methods are prohibited
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;  // parent = QModelIndex()
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;  // role = Qt::DisplayRole

    [[nodiscard]] double getProgress() const;
    void setProgress(double progress);

    Q_INVOKABLE int readSolcoreDb(const QString& db_path);
    Q_INVOKABLE int readDfDb(const QString& db_path);

signals:
    void progressChanged(double progress);

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    // If using QObject, the values should be a pointer
    QMap<QString, OpticMaterial *> m_list;

    QString db_name;
    double import_progress;
};

#endif  // SUISAPP_MATERIALDBMODEL_H
