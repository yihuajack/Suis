//
// Created by Yihua Liu on 2024-8-12.
//

#ifndef SUISAPP_SQLBROWSER_H
#define SUISAPP_SQLBROWSER_H

#include <QWidget>
#include <QSqlTableModel>

QT_FORWARD_DECLARE_CLASS(QSqlError)

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SqlBrowser;
}
QT_END_NAMESPACE

class SqlBrowser : public QWidget {
    Q_OBJECT

public:
    explicit SqlBrowser(QWidget *parent = nullptr);

    QSqlError addConnection(const QString &driver, const QString &dbName, const QString &host,
                            const QString &user, const QString &passwd, int port);

public slots:
    void openNewConnectionDialog();
    void about();

protected:
    void insertRow();
    void deleteRow();
    void updateActions();

protected slots:
    void exec();
    void showTable(const QString &table);
    void showMetaData(const QString &table);

    void onFieldStrategyAction();
    void onRowStrategyAction();
    void onManualStrategyAction();
    void onSubmitButton();
    void onClearButton();

signals:
    void statusMessage(const QString &message);

private:
    Ui::SqlBrowser *m_ui;
};

#endif  // SUISAPP_SQLBROWSER_H
