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

    // If making SqlTreeModel QML_SINGETON, then cannot use this declaration!
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
                id: treeView

                // Layout.column: 0
                // Layout.columnSpan: 1
                // Lauout.preferredWidth: 0.5 * parent.width
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                selectionModel: ItemSelectionModel { }  // by default

                model: sqlTreeModel

                delegate: TreeViewDelegate {
                    id: viewDelegate

                    readonly property real _padding: 5
                    readonly property real szHeight: contentItem.implicitHeight * 2.5
                    // required property int row, column, depth, hasChildren, bool current, expanded, isTreeNode
                    // required property TreeView treeView

                    implicitHeight: szHeight
                    implicitWidth: _padding + contentItem.x + contentItem.implicitWidth + _padding

                    background: Rectangle {  // Background rectangle enabled to show the alternative row colors
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
                        // https://stackoverflow.com/questions/69964071/qt-qml-colorimage-is-not-a-type
                        // ColorImage {
                        //     width: parent.width / 3
                        //     height: parent.height / 3
                        //     anchors.centerIn: parent
                        //     source: "qrc:/images/arrow_icon.png"
                        //     color: palette.buttonText
                        // }
                    }

                    contentItem: Label {
                        x: viewDelegate._padding + viewDelegate.depth + viewDelegate.indentation
                        width: parent.width - viewDelegate._padding - x
                        text: viewDelegate.model.display
                        elide: Text.ElideRight
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
