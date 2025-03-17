/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dbLoginDialog
    title: qsTr("Login")
    width: 400
    height: 400
    anchors.centerIn: parent
    modal: true

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel  // buttons: []
        onAccepted: {
            model.path = urlField.text
            let connected = SqlTreeModel.addConnection("QOCI", urlField.text, usernameField.text, passwordField.text)
            // Injection of parameters into signal handlers is deprecated.
            // Use JavaScript functions with formal parameters instead.
            if (connected) {
                dbLoginDialog.accept()
            } else {
                dbLoginDialog.reject()
            }
        }
        onRejected: {
            dbLoginDialog.reject()
        }
    }

    contentItem: ColumnLayout {
        anchors.centerIn: parent
        spacing: 10

        RowLayout {
            Label {
                text: "Username"
                Layout.alignment: Qt.AlignLeft
                width: 80
            }
            TextField {
                id: usernameField
                placeholderText: "Enter username"
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Label {
                text: "Password"
                Layout.alignment: Qt.AlignLeft
                width: 80
            }
            TextField {
                id: passwordField
                placeholderText: "Enter password"
                echoMode: TextInput.Password
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Label {
                text: qsTr("Database")
                Layout.alignment: Qt.AlignLeft
                width: 80
            }
            TextField {
                id: urlField
                placeholderText: "Enter database URL"
                Layout.fillWidth: true
            }
        }
    }
}
