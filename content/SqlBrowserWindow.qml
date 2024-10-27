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
                    sqlTreeModel.refreshAll();
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

    SqlTreeModel {
        id: sqlTreeModel
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

            TreeView {
                id: dbTreeView

                // Layout.column: 0
                // Layout.columnSpan: 1
                Layout.fillHeight: true
                // Lauout.preferredWidth: 0.5 * parent.width
                Layout.fillWidth: true
                clip: true

                selectionModel: ItemSelectionModel { }

                model: sqlTreeModel

                delegate: TreeViewDelegate {
                    id: viewDelegate

                    readonly property real _padding: 5
                    required property int column
                    required property bool current
                    required property int depth
                    required property bool expanded
                    required property int hasChildren
                    required property bool isTreeNode
                    required property int row
                    readonly property real szHeight: contentItem.implicitHeight * 2.5
                    required property TreeView treeView

                    implicitHeight: szHeight
                    implicitWidth: _padding + contentItem.x + contentItem.implicitWidth + _padding

                    background: Rectangle { // Background rectangle enabled to show the alternative row colors
                        id: background

                        anchors.fill: parent
                        color: {
                            if (viewDelegate.model.row === viewDelegate.treeView.currentRow) {
                                return Qt.lighter(palette.highlight, 1.2);
                            } else {
                                if (viewDelegate.treeView.alternatingRows && viewDelegate.model.row % 2 !== 0) {
                                    return (Application.styleHints.colorScheme === Qt.Light) ? Qt.darker(palette.alternateBase, 1.25) : Qt.lighter(palette.alternateBase, 2.);
                                } else {
                                    return palette.base;
                                }
                            }
                        }

                        Rectangle {
                            color: {
                                if (viewDelegate.model.row === viewDelegate.treeView.currentRow) {
                                    return (Application.styleHints.colorScheme === Qt.Light) ? Qt.darker(palette.highlight, 1.25) : Qt.lighter(palette.highlight, 2.);
                                } else {
                                    return "transparent";
                                }
                            }
                            height: parent.height
                            visible: !viewDelegate.model.column
                            // The selection indicator shown on the left side of the highlighted row
                            width: viewDelegate._padding
                        }
                    }

                    indicator: Item {
                        x: viewDelegate._padding + viewDelegate.depth * viewDelegate.indentation
                        implicitWidth: viewDelegate.szHeight
                        implicitHeight: viewDelegate.szHeight
                        visible: viewDelegate.isTreeNode && viewDelegate.hasChildren
                        rotation: viewDelegate.expanded ? 90 : 0
                        TapHandler {
                            onSingleTapped: {
                                let index = viewDelegate.treeView.index(viewDelegate.model.row, viewDelegate.model.column)
                                viewDelegate.treeView.selectionModel.setCurrentIndex(index, ItemSelectionModel.NoUpdate)
                                viewDelegate.treeView.toggleExpanded(viewDelegate.model.row)
                            }
                        }
                        ColorImage {
                            width: parent.width / 3
                            height: parent.height / 3
                            anchors.centerIn: parent
                            source: "qrc:/images/arrow_icon.png"
                            color: palette.buttonText
                        }
                    }

                    contentItem: Item {
                        x: viewDelegate._padding + (viewDelegate.depth + viewDelegate.indentation)
                        height: parent.height
                        width: parent.width - viewDelegate._padding - x

                        Label {
                            id: text_name
                            width: parent.width

                            elide: Text.ElideRight
                            text: viewDelegate.model.name
                        }

                        MouseArea {
                            // https://forum.qt.io/topic/31893/treeview-context-menu
                            // https://stackoverflow.com/questions/32448678/how-to-show-a-context-menu-on-right-click-in-qt5-5-qml-treeview
                            id: nodeArea

                            acceptedButtons: Qt.RightButton
                            anchors.fill: parent

                            onClicked: contextMenu.popup()

                            Menu {
                                id: contextMenu

                                MenuItem {
                                    text: "Refresh"

                                    onTriggered: SqlTreeModel.refresh(model.index)
                                }

                                MenuItem {
                                    enabled: isTreeNode && !hasChildren
                                    text: "Show Schema"

                                    onTriggered: sqlTableView.model = viewDelegate.model.table
                                }
                            }
                        }
                    }
                }
            }

            TableView {
                id: sqlTableView

                // Layout.column: 1
                // Layout.columnSpan: 1
                Layout.fillHeight: true
                // Layout.fillWidth: true
                Layout.preferredWidth: 0.5 * parent.width
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
