/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import content

// https://doc.qt.io/qt-6/qtqml-documents-structure.html
// https://youtrack.jetbrains.com/issue/CPP-42122/QML-Language-Support-Add-support-for-Qt-6.7-pragmas-including-ComponentBehavior-etc.
pragma ComponentBehavior: Bound

// Using a Component as the root of a QML document is deprecated:
// types defined in qml documents are automatically wrapped into Components when needed.
ApplicationWindow {
    id: root

    property var currentSqlTableModel: {}  // nothing = undefined

    height: 600
    width: 800
    title: qsTr("SQL Browser")
    visible: false
    color: Colors.background

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
                handle: Rectangle {
                    implicitWidth: 10
                    color: SplitHandle.pressed ? Colors.color2 : Colors.background
                    border.color: SplitHandle.hovered ? Colors.color2 : Colors.background
                    opacity: SplitHandle.hovered || sqlTreeView.width < 15 ? 1.0 : 0.0

                    Behavior on opacity {
                        OpacityAnimator {
                            duration: 1400
                        }
                    }
                }

                SqlBrowserTreeView {
                    id: sqlTreeView
                    color: Colors.surface1
                    SplitView.preferredWidth: 250
                    SplitView.fillHeight: true

                    onTableSelected: tableModel => root.currentSqlTableModel = tableModel
                }

                TableView {
                    id: sqlTableView

                    SplitView.fillWidth: true
                    SplitView.fillHeight: true

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
