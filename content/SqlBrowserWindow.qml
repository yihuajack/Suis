/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQml
import content

// https://doc.qt.io/qt-6/qtqml-documents-structure.html
// https://youtrack.jetbrains.com/issue/CPP-42122/QML-Language-Support-Add-support-for-Qt-6.7-pragmas-including-ComponentBehavior-etc.
pragma ComponentBehavior: Bound

// Using a Component as the root of a QML document is deprecated:
// types defined in qml documents are automatically wrapped into Components when needed.
ApplicationWindow {
    id: root

    property var currentSqlTableModel

    property string backendDir: ""

    height: 600
    width: 800
    title: qsTr("SQL Browser")
    visible: false
    color: Colors.background

    footer: Row {
        leftPadding: 10
        spacing: 10

        Text {
            text: qsTr("Queried Database Index")
            font.pixelSize: 16
            anchors.verticalCenter: parent.verticalCenter
        }

        TextField {
            id: queryDbIndex
            text: "0"
            onReleased: {
                SqlTreeModel.dbId = Number.fromLocaleString(Qt.locale(), queryDbIndex.text)
            }
        }

        Text {
            text: qsTr("Maximum Tables")
            font.pixelSize: 16
            anchors.verticalCenter: parent.verticalCenter
        }

        TextField {
            id: maxTablesIndex
            text: "3"
            onReleased: {
                SqlTreeModel.maxTables = Number.fromLocaleString(Qt.locale(), maxTablesIndex.text)
            }
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")

            MenuItem {
                id: addConnMenuItem
                text: qsTr("Add Connection...")

                onTriggered: {
                    dialogLoader.active = false;
                    dialogLoader.source = "DbLoginDialog.qml";
                    dialogLoader.active = true;
                }
            }

            MenuItem {
                id: importMenuItem
                text: qsTr("Import")
                enabled: false

                onTriggered: {
                    let read = SqlTreeModel.readGclDb(backendDir)
                    if (!read) {
                        console.log("Failed to read GCL database")
                    }
                }
            }

            MenuItem {
                id: refreshAllMenuItem
                text: qsTr("Refresh All")
                enabled: false

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

        onLoaded: {
            if (item) {
                item.open()

                item.accepted.connect(function() {
                    importMenuItem.enabled = true
                    refreshAllMenuItem.enabled = true
                })
            }
        }
    }

    // https://stackoverflow.com/questions/69845356/what-is-the-qmessageboxaboutqt-equivalent-in-qml
    MessageDialog {
        id: aboutDialog

        text: qsTr("The SQL Browser shows how a data browser can be used to visualize the results of SQL statements on a live database.")
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

                    // Cannot assign QSharedPointer<SqlTableModel> to SqlTableModel*
                    onTableSelected: tableModel => root.currentSqlTableModel = tableModel
                }

                Rectangle {
                    color: Colors.surface1  // Application.styleHints.appearance === Qt.Light ? palette.mid : palette.midlight

                    SplitView.fillWidth: true
                    SplitView.fillHeight: true

                    HorizontalHeaderView {
                        id: horizontalHeader
                        anchors.left: sqlTableView.left
                        anchors.top: parent.top
                        syncView: sqlTableView
                        clip: true
                    }

                    VerticalHeaderView {
                        id: verticalHeader
                        anchors.top: sqlTableView.top
                        anchors.left: parent.left
                        syncView: sqlTableView
                        clip: true
                    }

                    TableView {
                        id: sqlTableView
                        anchors.left: verticalHeader.right
                        anchors.top: horizontalHeader.bottom
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        columnWidthProvider: function() { return 100; }
                        rowHeightProvider: function() { return 60; }
                        ScrollBar.horizontal: ScrollBar { }
                        ScrollBar.vertical: ScrollBar { }
                        clip: true

                        model: root.currentSqlTableModel

                        // implicitWidth and implicitHeight readonly for Text
                        // https://forum.qt.io/topic/159763/tableview-with-qsqlrelationaltablemodel-cells-are-all-2
                        delegate: ItemDelegate {
                            required property var model
                            text: model.display
                            padding: 10
                            font.pixelSize: 15
                        }

                        ScrollIndicator.horizontal: ScrollIndicator { }
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }
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

                onEditingFinished: {
                    SqlTreeModel.execQuery(queryText.text)
                }
            }
        }
    }
}
