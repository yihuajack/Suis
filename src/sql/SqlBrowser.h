//
// Created by Yihua Liu on 2024-8-12.
//

#ifndef SUISAPP_SQLBROWSER_H
#define SUISAPP_SQLBROWSER_H

#include <QSqlTableModel>

QT_FORWARD_DECLARE_CLASS(QSqlError)

class SqlBrowser : public QObject {
    Q_OBJECT

public:
    explicit SqlBrowser(QObject *parent = nullptr);

    QSqlError addConnection(const QString &driver, const QString &dbName, const QString &host,
                            const QString &user, const QString &passwd, int port);

};

#endif  // SUISAPP_SQLBROWSER_H
