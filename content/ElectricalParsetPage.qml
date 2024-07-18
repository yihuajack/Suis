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
                    width: devLView.width
                    height: 150

                    property string databasePath: ""
                    // Warning: it is a one-way binding rather than two-way binding!
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
                                text: databasePath
                                onEditingFinished: {
                                    if (text.length > 0) {
                                        importDev(text)
                                    }
                                }
                            }

                            Text {
                                id: devName
                                text: ""
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
                                    ratChartLoader.active = false
                                    ratChartLoader.source = "DeviceTableDialog.qml"
                                    ratChartLoader.active = true
                                }
                            }

                            Button {
                                id: showRATButton
                                text: "Show RAT"
                                enabled: false
                                onClicked: {
                                    device.calcRAT()
                                    ratChartLoader.active = false
                                    ratChartLoader.source = "OpticalDeviceDialog.qml"
                                    ratChartLoader.active = true
                                }
                            }

                            Button {
                                id: removeButton
                                text: "Remove"
                                onClicked: {
                                    DevSysModel.removeDevice(index)
                                }
                            }
                        }
                    }

                    FileDialog {
                        id: devFileDialog
                        title: qsTr("Select Device File")
                        onAccepted: {
                            importDev(selectedFile)
                        }
                    }

                    Loader {
                        id: devDialogLoader
                        asynchronous: true

                        onLoaded: item.open()
                    }

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
                        showRATButton.enabled = status === true
                        if (status) {
                            devName.text = device.name
                        }
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
