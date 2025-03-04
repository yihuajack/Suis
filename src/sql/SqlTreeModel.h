//
// Created by Yihua Liu on 2024/8/12.
//

#ifndef SUISAPP_SQLTREEMODEL_H
#define SUISAPP_SQLTREEMODEL_H

#include <QAbstractItemModel>
#include <QQmlEngine>

#include "SqlTreeItem.h"

class SqlTreeModel : public QAbstractItemModel {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(int dbId READ dbId WRITE setDbId NOTIFY dbIdChanged)
    Q_PROPERTY(int maxTables READ maxTables WRITE setMaxTables NOTIFY maxTablesChanged)

public:
    Q_DISABLE_COPY_MOVE(SqlTreeModel);

    enum ModelRoles {
        SqlTableRole = Qt::UserRole + 1
    };

    explicit SqlTreeModel(QObject *parent = nullptr);
    ~SqlTreeModel() override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;  // role = Qt::DisplayRole
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;  // parent = {}

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;  // role = Qt::EditRole
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;  // role = Qt::EditRole

    bool insertColumns(int position, int columns, const QModelIndex &parent) override;  // parent = {}
    bool removeColumns(int position, int columns, const QModelIndex &parent) override;  // parent = {}
    bool insertRows(int position, int rows, const QModelIndex &parent = {}) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = {}) override;

    Q_INVOKABLE bool addConnection(const QString &driver, const QString &database, const QString &user,
                                   const QString &passwd);
    Q_INVOKABLE void refresh(const QModelIndex &current);
    Q_INVOKABLE void refreshAll();
    Q_INVOKABLE void execQuery(const QString &query) const;
    Q_INVOKABLE bool upload(const QString &path) const;

    [[nodiscard]] int dbId() const;
    void setDbId(int dbId);
    [[nodiscard]] int maxTables() const;
    void setMaxTables(int dbId);

signals:
    void dbIdChanged();
    void maxTablesChanged();

protected:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    [[nodiscard]] SqlTreeItem *getItem(const QModelIndex &index) const;

    std::unique_ptr<SqlTreeItem> rootItem;

    int m_dbId = 0;
    int m_maxTables = 3;
};


#endif  // SUISAPP_SQLTREEMODEL_H
