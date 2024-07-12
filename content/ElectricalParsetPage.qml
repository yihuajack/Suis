/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls  // Dialog
import QtQuick.Dialogs
import QtQuick.Layouts
import content

ElectricalParsetPageForm {
    ScrollView {
        anchors.fill: parent

        ColumnLayout {
            spacing: 10
            // Must set left, right, top before setting margins!
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            anchors.topMargin: 20
            width: parent.width

            ListView {
                id: devLView
                Layout.alignment: Qt.AlignTop  // | Qt.AlignLeft
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
                model: DevSysModel

                delegate: Item {
                    width: parent.width
                    height: 150

                    property string databasePath: ""
                    property var device: model.device

                    Column {
                        spacing: 10
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.topMargin: 10

                        Row {
                            spacing: 10

                            TextField {
                                id: devPathTextField
                                width: 600
                                // Do not add width: parent.width! Otherwise,
                                // QML Row: possible QQuickItem::polish() loop
                                // QML Row: Row called polish() inside updatePolish() of Row
                                placeholderText: "Enter the Device Path"
                                onEditingFinished: {
                                    if (text.length > 0) {
                                        importDev(text)
                                    }
                                }
                            }

                            Text {
                                id: statusIcon
                            }
                        }

                        Row {
                            spacing: 10

                            Button {
                                id: devImportButton
                                text: "Import"
                                onClicked: {
                                    devFileDialog.nameFilters = ["*csv"]
                                    devFileDialog.open()
                                }
                            }

                            Button {
                                id: editTableButton
                                text: "Edit Table"
                                enabled: false
                                onClicked: {
                                    devTableDialog.open()
                                }
                            }

                            Button {
                                id: showRATButton
                                text: "Show RAT"
                                enabled: false
                                onClicked: {
                                    ratChartLoader.source = "OpticalDeviceDialog.qml"
                                }
                            }

                            Button {
                                id: removeButton
                                text: "Remove"
                                enabled: false
                                onClicked: {
                                    model.removeDevice(index)
                                }
                            }
                        }
                    }

                    FileDialog {
                        id: devFileDialog
                        title: qsTr("Select Database File")
                        onAccepted: {
                            importDev(selectedFile)
                        }
                    }

                    // Dialog {
                    //     id: devTableDialog
                    //     title: "Imported Electrical Materials"
                    //     width: parent.width * 0.8
                    //     height: parent.width * 0.6
                    //     anchors.centerIn: parent
                    //     modal: true
                    //     standardButtons: Dialog.Ok
                    //
                    //     contentItem: TableView {
                    //         id: devTView
                    //         width: parent.width
                    //         height: parent.height - 20
                    //     }
                    // }

                    Loader {
                        id: ratChartLoader
                        asynchronous: true

                        onLoaded: item.open()
                    }

                    function importDev(dbPath) {
                        if (dbPath !== databasePath) {
                            databasePath = dbPath.toString()
                        }
                        let status = device.readDfDev(databasePath)
                        editTableButton.enabled = status === true
                        statusIcon.text = status ? "√" : "×"
                    }
                }
            }

            Button {
                id: addButton
                // Layout.alignment: Qt.AlignTop | Qt.AlignHCenter does no take effects; give up!
                Layout.bottomMargin: 20
                text: "Add Device"
                onClicked: {
                    DevSysModel.addDevice();
                }
            }
        }
    }
}
