/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls  // Dialog
import QtQuick.Dialogs
import QtCore  // StandardPaths: QtLabsPlatform is deprecated since 6.4
import MaterialDbModel  // Qt.labs.folderlistmodel

OpticalParsetPageForm {
    id: opticalParsetPage

    MaterialDbModel {
        id: matDbModel
    }

    ListModel {
        id: listModel
        ListElement {
            name: "Solcore"
            checked: false
        }

        ListElement {
            name: "Df"
            checked: false
        }
    }

    ListView {
        anchors.fill: parent
        model: listModel
        delegate: Item {
            width: parent.width
            height: 200

            property string databasePath: ""
            property var matList: []

            Row {
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter

                CheckBox {
                    id: checkSolcore
                    text: "Solcore"
                    onCheckedChanged: {
                        model.checked = checked
                    }
                }

                Column {
                    width: 400
                    spacing: 10

                    TextField {
                        id: pathTextField
                        width: 400
                        enabled: model.checked
                        placeholderText: "Enter the Database Path"
                        text: databasePath
                        onTextChanged: {
                            model.pathText = text
                        }
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
                        if (model.name === "Solcore") {
                            fileDialog.nameFilters = ["*.ini", "*.txt"]
                        } else if (model.name === "Df") {
                            fileDialog.nameFilters = ["*.xlsx"]
                        }
                        fileDialog.open()
                    }
                }

                Button {
                    id: showButton
                    text: "Show"
                    enabled: model.status === 1
                    onClicked: {
                        matListDialog.open()
                    }
                }

                FileDialog {
                    id: fileDialog
                    title: qsTr("Select Database File")
                    onAccepted: {
                        databasePath = selectedFile
                        let result
                        if (model.name === "Solcore") {
                            result = matDbModel.readSolcoreDb(databasePath)
                        } else if (model.name === "Df") {
                            result = matDbModel.readDfDb(databasePath)
                        }
                        matList = result.matlist
                        showButton.enabled = true
                        statusText.text = statusInfo(result.status)
                    }
                }

                Dialog {
                    id: matListDialog
                    title: "Imported Optical Materials"
                    standardButtons: Dialog.Ok

                    Column {
                        spacing: 10
                        padding: 10

                        ListView {
                            width: parent.width
                            height: parent.height - 50
                            model: matList

                            delegate: Item {
                                width: parent.width
                                height: 30

                                Text {
                                    text: modelData
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    function statusInfo(status) {
        switch (status) {
            case 0:
                return "Database is successfully imported"
            case 1:
                return "Cannot find the path"
            case 2:
                return "Fail to load the n/k file"
            default:
                return "Invalid status"
        }
    }
}
