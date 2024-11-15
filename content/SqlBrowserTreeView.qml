/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import content

pragma ComponentBehavior: Bound

Rectangle {
    id: root

    signal tableSelected(var tableModel)

    TreeView {
        id: treeView

        property int lastIndex: -1

        // Layout.column: 0
        // Layout.columnSpan: 1
        // Layout.preferredWidth: 0.5 * parent.width
        anchors.fill: parent
        clip: true

        selectionModel: ItemSelectionModel { }  // by default

        model: SqlTreeModel

        delegate: TreeViewDelegate {
            id: viewDelegate

            readonly property real _padding: 0
            readonly property real szHeight: contentItem.implicitHeight * 2.5
            // required property int row, column, depth, hasChildren, bool current, expanded, isTreeNode
            // required property TreeView treeView
            // Since we have the 'ComponentBehavior Bound' pragma, we need to
            // require these properties from our model. This is a convenient way
            // to bind the properties provided by the model's role names.
            required property int index
            // required property var tableModel  // Required properties with initializer do not make sense.

            implicitHeight: szHeight
            implicitWidth: _padding + contentItem.x + contentItem.implicitWidth + _padding

            background: Rectangle {
                color: (viewDelegate.index === treeView.lastIndex)
                    ? Colors.selection
                    : (hoverHandler.hovered ? Colors.active : "transparent")
            }

            // https://stackoverflow.com/questions/69964071/qt-qml-colorimage-is-not-a-type
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
                Image {
                    id: arrowIcon
                    anchors.verticalCenter: parent.verticalCenter
                    source: "images/arrow_icon.png"
                    sourceSize.width: 20
                    sourceSize.height: 20
                    fillMode: Image.PreserveAspectFit

                    smooth: true
                    antialiasing: true
                    asynchronous: true
                }
            }

            contentItem: Label {
                x: viewDelegate._padding + viewDelegate.depth + viewDelegate.indentation
                width: parent.width - viewDelegate._padding - x
                text: viewDelegate.model.display
                elide: Text.ElideRight
                color: Colors.text
            }

            // We color the directory icons with this MultiEffect, where we overlay
            // the colorization color ontop of the SVG icons.
            // https://forum.qt.io/topic/159736/qml-multieffect-garbled-error-message
            MultiEffect {
                id: iconOverlay

                anchors.fill: indicator.arrowIcon
                source: arrowIcon
                colorization: 1.0
                brightness: 1.0
                colorizationColor: {
                    const isTable = viewDelegate.index === treeView.lastIndex && !viewDelegate.hasChildren;
                    if (isTable) {
                        return Qt.lighter(Colors.database, 3)
                    }

                    const isExpanded = viewDelegate.expanded && viewDelegate.hasChildren;
                    return isExpanded ? Colors.color2 : Colors.database
                }
            }

            HoverHandler {
                id: hoverHandler
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
                                root.tableSelected(viewDelegate.model.table)
                            }
                            break;
                        case Qt.RightButton:
                            if (viewDelegate.hasChildren) {
                                contextMenu.popup();
                            }
                            break;
                    }
                }
            }

            ContextMenu {
                id: contextMenu
                Action {
                    text: qsTr("Set as root index")
                    onTriggered: {
                        treeView.rootIndex = treeView.index(viewDelegate.row, 0)
                    }
                }
                Action {
                    text: qsTr("Reset root index")
                    onTriggered: treeView.rootIndex = undefined
                }
            }
        }

        // Provide our own custom ScrollIndicator for the TreeView.
        ScrollIndicator.vertical: ScrollIndicator {
            active: true
            implicitWidth: 15

            contentItem: Rectangle {
                implicitWidth: 6
                implicitHeight: 6

                color: Colors.color1
                opacity: sqlTreeView.movingVertically ? 0.5 : 0.0

                Behavior on opacity {
                    OpacityAnimator {
                        duration: 500
                    }
                }
            }
        }
    }
}
