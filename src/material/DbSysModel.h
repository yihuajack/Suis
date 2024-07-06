//
// Created by Yihua Liu on 2024-06-17.
//

#ifndef SUISAPP_DBSYSMODEL_H
#define SUISAPP_DBSYSMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

#include "MaterialDbModel.h"

class DbSysModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum DbSysRoles {
        NameRole = Qt::UserRole + 1,
        CheckedRole,
        PathRole,
        ProgressRole,
        ModelRole
    };

    explicit DbSysModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void addModel(MaterialDbModel *db_model);

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public slots:
    void onProgressChanged();

private:
    QList<MaterialDbModel *> m_db;
};

#endif  // SUISAPP_DBSYSMODEL_H
