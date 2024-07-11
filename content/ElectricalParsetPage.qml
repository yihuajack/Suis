/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls  // Dialog
import QtQuick.Dialogs
import QtCore  // StandardPaths: QtLabsPlatform is deprecated since 6.4
import content

ElectricalParsetPageForm {
    ListView {
        width: 800
        height: 600
        model: DevSysModel
        delegate: Item {
            width: parent.width
            height: 100

            property string databasePath: ""

            Row {
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter

                TextField {
                    id: devPathTextField
                    // Do not add width: parent.width! Otherwise,
                    // QML Row: possible QQuickItem::polish() loop
                    // QML Row: Row called polish() inside updatePolish() of Row
                    placeholderText: "Enter the Device Path"
                    text: ""
                    onEditingFinished: {
                        if (text.length > 0) {
                            importDev(text)
                        }
                    }
                }

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
                let status = devModel.readDfDev(databasePath)
                editTableButton.enabled = status === true
            }
        }
    }

    Row {
        Button {
            id: addButton
            text: "Add Device"
            onClicked: {
                DevSysModel.addDevice();
            }
        }
    }
}
