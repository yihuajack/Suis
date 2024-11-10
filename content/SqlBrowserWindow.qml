/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import content

// Using a Component as the root of a QML document is deprecated:
// types defined in qml documents are automatically wrapped into Components when needed.
ApplicationWindow {
    id: root

    property var currentSqlTableModel

    height: 600
    width: 800
    title: qsTr("SQL Browser")
    visible: false

    footer: Text {
        id: sqlStatusText

        color: "steelblue"
    }
    menuBar: MenuBar {
        Menu {
            title: qsTr("File")

            MenuItem {
                text: qsTr("Add Connection...")

                onTriggered: {
                    dialogLoader.active = false;
                    dialogLoader.source = "DbLoginDialog.qml";
                    dialogLoader.active = true;
                }
            }
            MenuItem {
                text: qsTr("Refresh All")

                onTriggered: {
                    SqlTreeModel.refreshAll();
                }
            }
            MenuSeparator {
            }
            MenuItem {
                text: "Quit"

                onTriggered: close()
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

    Loader {
        id: dialogLoader

        asynchronous: true

        onLoaded: item.open()
    }

    // https://stackoverflow.com/questions/69845356/what-is-the-qmessageboxaboutqt-equivalent-in-qml
    MessageDialog {
        id: aboutDialog

        text: qsTr("The SQL Browser shows how a data browser can be used to visualize the results of SQL statements on a live database")
        title: qsTr("About")
    }

    ColumnLayout {
        anchors.fill: parent

        // rows: 2
        // columns: 2
        // rowSpacing: 10
        // columnSpacing: 10
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 0.7 * parent.height

            SplitView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                SqlBrowserTreeView {
                    id: sqlTreeView

                    onTableSelected: tableModel => root.currentSqlTableModel = tableModel
                }

                TableView {
                    id: sqlTableView

                    // Layout.column: 1
                    // Layout.columnSpan: 1
                    Layout.fillHeight: true
                    // Layout.fillWidth: true
                    Layout.preferredWidth: 0.5 * parent.width

                    model: root.currentSqlTableModel
                }
            }
        }

        GroupBox {
            // Layout.row: 1
            // Layout.column: 0
            // Layout.columnSpan: 2
            // Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredHeight: 0.3 * parent.height
            title: qsTr("SQL Query")

            TextEdit {
                id: queryText

                anchors.fill: parent
            }
        }
    }
}
