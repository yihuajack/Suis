//
// Created by Yihua Liu on 2024/4/8.
//

#ifndef SUISAPP_DBMODEL_H
#define SUISAPP_DBMODEL_H

#include <filesystem>
#include <QAbstractListModel>

#include "OpticMaterial.h"

class DbModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(QString dbPath READ dbPath WRITE setDbPath NOTIFY dbPathChanged)

private:
    QString m_db_name;
    QMap<QString, OpticMaterial> m_list;
    QString m_db_path;

    void import();

public:
    enum Roles {
        MatNameRole = Qt::UserRole
    };

    explicit DbModel(QString db_name);
    [[nodiscard]] int rowCount(const QModelIndex &) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QString dbPath() const;
    void setDbPath(const QString& db_path);

signals:
    void dbPathChanged();

public slots:
    void readDb();
};


#endif  // SUISAPP_DBMODEL_H
