//
// Created by Yihua Liu on 2024/8/12.
//

#include "SqlTreeModel.h"

void SqlTreeModel::refresh() {
    const auto qDBCaption = [](const QSqlDatabase &db) {
        QString nm = db.driverName() + QLatin1Char(':');
        if (not db.userName().isEmpty())
            nm += db.userName() + QLatin1Char('@');
        nm += db.databaseName();
        return nm;
    };

    const QStringList connectionNames = QSqlDatabase::connectionNames();

    bool gotActiveDb = false;
    for (const QString& connectionName : connectionNames) {
        QSqlDatabase db = QSqlDatabase::database(connectionName, false);
    }
}

//void SqlModel::upload() {
//    if (connect()) {
//        QSqlQuery query;
//        query.prepare("INSERT INTO AI_STATS (BATTERY_SN, column2) VALUES (:value1, :value2)");
//    }
//}
