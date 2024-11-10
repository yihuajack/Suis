/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import content

Rectangle {
    id: root

    signal tableSelected(var tableModel)

    TreeView {
        id: treeView

        // Layout.column: 0
        // Layout.columnSpan: 1
        // Lauout.preferredWidth: 0.5 * parent.width
        clip: true

        selectionModel: ItemSelectionModel { }  // by default

        model: SqlTreeModel

        delegate: TreeViewDelegate {
            id: viewDelegate

            readonly property real _padding: 5
            readonly property real szHeight: contentItem.implicitHeight * 2.5
            // required property int row, column, depth, hasChildren, bool current, expanded, isTreeNode
            // required property TreeView treeView
            required property var tableModel

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

            TapHandler {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onSingleTapped: (eventPoint, button) => {
                    switch (button) {
                        case Qt.LeftButton:
                            treeView.toggleExpanded(viewDelegate.row)
                            treeView.lastIndex = viewDelegate.index
                            // If this model item doesn't have children, it means it's representing a table.
                            if (!viewDelegate.hasChildren) {
                                root.tableSelected(viweDelegate.tableModel)
                            }
                            break;
                        case Qt.RightButton:
                            if (treeDelegate.hasChildren) {
                                contextMenu.popup();
                            }
                            break;
                    }
                }
            }
        }
    }
}
