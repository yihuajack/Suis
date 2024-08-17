/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import SqlBrowser

// Using a Component as the root of a QML document is deprecated:
// types defined in qml documents are automatically wrapped into Components when needed.
ApplicationWindow {
    id: root
    width: 400
    height: 300
    visible: false
    title: "New Window with Menu"

    SqlBrowser {
        id: sqlBrowser
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("Add Connection...")
                onTriggered: console.log("New File")
            }

            MenuSeparator {}

            MenuItem {
                text: "Quit"
                onTriggered: close()
            }
        }

        Menu {
            title: "About"
            MenuItem {
                text: "Cut"
                onTriggered: console.log("Cut")
            }
            MenuItem {
                text: "Copy"
                onTriggered: console.log("Copy")
            }
            MenuItem {
                text: "Paste"
                onTriggered: console.log("Paste")
            }
        }

        Menu {
            title: "Help"
            MenuItem {
                text: "About"
                onTriggered: aboutDialog.open()
            }
        }
    }

    MessageDialog {
        id: aboutDialog
        title: qsTr("About")
        text: qsTr("The SQL Browser shows how a data browser can be used to visualize the results of SQL statements on a live database")
    }
}
