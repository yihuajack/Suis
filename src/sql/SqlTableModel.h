//
// Created by Yihua Liu on 2024-8-12.
//

#ifndef SUISAPP_SQLTABLEMODEL_H
#define SUISAPP_SQLTABLEMODEL_H

#include <QAbstractItemModel>
#include <QtSql/QSqlRelationalTableModel>

class SqlTableModel : public QSqlRelationalTableModel {
    Q_OBJECT

//    Q_PROPERTY(QAbstractItemModel model READ getModel CONSTANT)

public:
    explicit SqlTableModel(QObject *parent = nullptr);

//    [[nodiscard]] QAbstractItemModel getModel() const;


private:
    QAbstractItemModel *model;
};

#endif  // SUISAPP_SQLTABLEMODEL_H
