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
    // property var elecDbPaths: {}

    ListModel {
        id: devSysModel
        ListElement {
            name: "Df"
            checked: false
        }
    }

    ListView {
        width: 800
        height: 600
        model: devSysModel
        delegate: Item {
            width: parent.width
            height: 100

            property string databasePath: ""

            DeviceModel {
                id: devModel
            }

            Row {
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter

                CheckBox {
                    width: 100
                    text: model.name
                    onCheckedChanged: {
                        model.checked = checked
                        // if (elecDbPaths[model.name]) {
                        //     databasePath = elecDbPaths[model.name]
                        //     importDev(databasePath)
                        // }
                    }
                }

                Column {
                    width: parent.width * 0.6
                    spacing: 10

                    TextField {
                        id: pathTextField
                        width: parent.width
                        readOnly: !model.checked
                        placeholderText: "Enter the Database Path"
                        text: databasePath
                        onEditingFinished: {  // method inherited from TextInput
                            if (text.length > 0) {
                                importDev(text)
                            }
                        }
                    }

                    ProgressBar {
                        id: importProgressBar
                        width: parent.width
                        to: 1.0
                        value: devModel.progress
                    }

                    Text {
                        id: statusText
                        text: ""
                    }
                }

                Button {
                    id: importButton
                    text: "Import"
                    enabled: model.checked
                    onClicked: {
                        if (model.name === "Df") {
                            fileDialog.nameFilters = ["*csv"]
                        }
                        fileDialog.open()
                    }
                }

                Button {
                    id: showButton
                    text: "Show"
                    enabled: false
                    onClicked: {
                        matListDialog.open()
                    }
                }

                FileDialog {
                    id: fileDialog
                    title: qsTr("Select Database File")
                    onAccepted: {
                        importDev(selectedFile)
                    }
                }
            }

            function importDev(dbPath) {
                statusText.text = "Importing optical materials from database"
                databasePath = dbPath
                let status
                if (model.name === "Df") {
                    status = devModel.readDfDb(databasePath)
                }
                statusText.text = statusInfo(status)
                showButton.enabled = status === 0
            }
        }
    }

    function statusInfo(status) {
        switch (status) {
            case 0:
                return "Device is successfully imported"
            case 1:
                return "Cannot find the path"
            case 2:
                return "Device file content format is invalid"
            default:
                return "Invalid status"
        }
    }

    function updateDbPaths(optDbPaths) {
        elecDbPaths = optDbPaths;
    }
}
