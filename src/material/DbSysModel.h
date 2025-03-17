//
// Created by Yihua Liu on 2024-06-17.
//

#ifndef SUISAPP_DBSYSMODEL_H
#define SUISAPP_DBSYSMODEL_H

#include <QQmlEngine>

#include "MaterialDbModel.h"

class DbSysModel : public QAbstractListModel {
    Q_OBJECT
    QML_SINGLETON

public:
    enum DbSysRoles {
        NameRole = Qt::UserRole + 1,
        CheckedRole,
        PathRole,
        ProgressRole,
        ModelRole
    };

    QList<MaterialDbModel *> m_db;

    explicit DbSysModel(QObject *parent = nullptr);
    // https://stackoverflow.com/questions/50073626/reference-to-qml-singleton-class-instance
    static DbSysModel *instance();
    // static DbSysModel *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void addModel(MaterialDbModel *db_model);

    [[nodiscard]] OpticMaterial<QList<double>> *getMatByName(const QString &mat_name) const;

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public slots:
    void onProgressChanged();

private:
    static DbSysModel *m_instance;
};

#endif  // SUISAPP_DBSYSMODEL_H
