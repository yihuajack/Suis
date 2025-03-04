/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls

Dialog {
    id: devTableDialog
    title: "Imported Electrical Materials"
    width: 800
    height: 600
    anchors.centerIn: parent
    modal: true
    standardButtons: Dialog.Ok

    contentItem: Item {
        implicitWidth: devTableDialog.width  // do not use parent.width/height or anchors.fill: parent!
        implicitHeight: devTableDialog.height
        // The background color will show through the cell
        // spacing, and therefore become the grid line color.

        Rectangle {
            // https://forum.qt.io/topic/157764/qml-tableview-not-showing
            anchors.fill: parent
            color: Qt.styleHints.appearance === Qt.Light ? palette.mid : palette.midlight

            // Qt 6 no longer has TableViewColumn!
            VerticalHeaderView {
                id: verticalHeader
                anchors.top: devTView.top
                anchors.left: parent.left
                syncView: devTView
                clip: true
            }

            TableView {
                id: devTView
                anchors.left: verticalHeader.right  // remember to set all anchors!
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                clip: true

                columnSpacing: 1
                rowSpacing: 1

                selectionModel: ItemSelectionModel { }

                model: device

                // QML QQuickRectangle: TableView: detected anchors on delegate with index: .
                // Use implicitWidth and implicitHeight instead.
                delegate: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 50
                    color: palette.base  // border.color: "black"

                    // While the edit delegate is showing, the cell underneath will still be visible, and therefore
                    // shine through if the edit delegate is translucent, or otherwise doesn't cover the whole cell.
                    // If this is not wanted, you can either let the root item of the edit delegate be a solid
                    // Rectangle, or hide some of the items inside the TableView delegate. The latter can be done by
                    // defining a property required property bool editing inside it, that you bind to the visible
                    // property of some of the child items.
                    required property bool editing

                    Text {
                        id: textField
                        anchors.centerIn: parent
                        anchors.margins: 5
                        text: display  // model.display
                        visible: !editing
                    }

                    TableView.editDelegate: TextField {
                        anchors.fill: parent
                        text: display
                        horizontalAlignment: TextInput.AlignHCenter
                        verticalAlignment: TextInput.AlignVCenter
                        Component.onCompleted: selectAll()  // Select all text of text field when editing

                        // Note that in qtdeclarative/src/quick/doc/snippets/qml/tableview/editdelegate.qml
                        // 'display = text' is shorthand for:
                        // let index = TableView.view.index(row, column)
                        // For branch 6.7.2: TableView.view.model.setData(index, text, Qt.DisplayRole)
                        // For branch 6.8:   TableView.view.model.setData(index, "display", text)
                        TableView.onCommit: {
                            console.log("Table view commiting")
                            display = text  // edit = text
                            // TableView.view.model.setData(index, text, Qt.EditRole)
                        }
                    }
                }
            }

            SelectionRectangle {
                target: devTView
            }
        }
    }
}
