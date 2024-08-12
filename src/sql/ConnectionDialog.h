//
// Created by Yihua Liu on 2024-8-12.
//

#ifndef SUISAPP_CONNECTIONDIALOG_H
#define SUISAPP_CONNECTIONDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ConnectionDialogUi;
}
QT_END_NAMESPACE

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    ConnectionDialog(QWidget *parent = nullptr);

    QString driverName() const;
    QString databaseName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    int port() const;
    bool useInMemoryDatabase() const;

private slots:
    void onOkButton();
    void onDbCheckBox();

private:
    Ui::ConnectionDialogUi *m_ui;
};

#endif //SUISAPP_CONNECTIONDIALOG_H
