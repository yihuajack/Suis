//
// Created by Yihua Liu on 2024-8-12.
//

#include "SqlBrowser.h"

#include <QAction>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

SqlBrowser::SqlBrowser(QObject *parent) {
    if (QSqlDatabase::drivers().isEmpty()) {
        qWarning() << tr("No database drivers found")
                   << tr("This part requires at least one Qt database driver. "
                         "Please check the documentation how to build the "
                         "Qt SQL plugins.");
    }
}

QSqlError SqlBrowser::addConnection(const QString &driver, const QString &dbName, const QString &host,
                                 const QString &user, const QString &passwd, int port) {
    static int cCount = 0;

    QSqlError err;
    QSqlDatabase db = QSqlDatabase::addDatabase(driver, QString("SqlBrowser%1").arg(++cCount));
    db.setDatabaseName(dbName);
    db.setHostName(host);
    db.setPort(port);
    if (not db.open(user, passwd)) {
        err = db.lastError();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(QString("SqlBrowser%1").arg(cCount));
    }
//    m_ui->connectionWidget->refresh();

    return err;
}
