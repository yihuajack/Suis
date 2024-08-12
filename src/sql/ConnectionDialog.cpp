//
// Created by Yihua Liu on 2024-8-12.
//

#include "ConnectionDialog.h"

#include <ui_connectiondialog.h>

#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlDatabase>

ConnectionDialog::ConnectionDialog(QWidget *parent)
        : QDialog(parent)
        , m_ui(new Ui::ConnectionDialogUi) {
    m_ui->setupUi(this);

    QStringList drivers = QSqlDatabase::drivers();

    if (not drivers.contains("QSQLITE")) {
        m_ui->dbCheckBox->setEnabled(false);
    }

    m_ui->comboDriver->addItems(drivers);

    connect(m_ui->okButton, &QPushButton::clicked,
            this, &ConnectionDialog::onOkButton);
    connect(m_ui->cancelButton, &QPushButton::clicked,
            this, &ConnectionDialog::reject);
    connect(m_ui->dbCheckBox, &QCheckBox::stateChanged,
            this, &ConnectionDialog::onDbCheckBox);
}

QString ConnectionDialog::driverName() const {
    return m_ui->comboDriver->currentText();
}

QString ConnectionDialog::databaseName() const {
    return m_ui->editDatabase->text();
}

QString ConnectionDialog::userName() const {
    return m_ui->editUsername->text();
}

QString ConnectionDialog::password() const {
    return m_ui->editPassword->text();
}

QString ConnectionDialog::hostName() const {
    return m_ui->editHostname->text();
}

int ConnectionDialog::port() const {
    return m_ui->portSpinBox->value();
}

bool ConnectionDialog::useInMemoryDatabase() const {
    return m_ui->dbCheckBox->isChecked();
}

void ConnectionDialog::onOkButton() {
    if (m_ui->comboDriver->currentText().isEmpty()) {
        QMessageBox::information(this, tr("No database driver selected"),
                                 tr("Please select a database driver"));
        m_ui->comboDriver->setFocus();
    } else {
        accept();
    }
}

void ConnectionDialog::onDbCheckBox() {
    m_ui->connGroupBox->setEnabled(!m_ui->dbCheckBox->isChecked());
}
