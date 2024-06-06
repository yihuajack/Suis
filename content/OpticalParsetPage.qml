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
            status: 0
        }

        ListElement {
            name: "Df"
            checked: false
            status: 0
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

                TextField {
                    id: pathTextField
                    width: 400
                    placeholderText: "Enter the Database Path"
                    text: databasePath
                    onTextChanged: {
                        model.pathText = text
                    }
                }

                Button {
                    id: importButton
                    text: "Import"
                    enabled: model.checked
                    onClicked: {
                        if (model.name === "Solcore") {  // maybe better than using index
                            folderDialog.open()
                        } else if (model.name === "Df") {
                            fileDialog.open()
                        }
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

                Text {
                    id: statusText
                    left: pathTextField.left
                    top: pathTextField.bottom - 10
                    width: pathTextField.width
                    height: pathTextField.height
                    text: ""
                }

                FolderDialog {
                    id: folderDialog
                    title: qStr("Select Database Folder")
                    onAccepted: {
                        if (model.name === "Solcore") {  // folderDialogLoader.sourceComponent = folderDialogComponent;
                            databasePath = selectedFolder
                            matList = matDbModel.readSolcoreDb(databasePath)
                            model.status = matList.length !== 0
                            showButton.enabled = true
                            statusText.text = statusInfo(model.status)
                        }
                    }
                }

                FileDialog {
                    id: fileDialog
                    titile: qStr("Select Database File")
                    nameFilters: ["*.xlsx"]
                    onAccepted: {
                        if (model.name === "Df") {
                            databasePath = selectedFile
                            model.status = matDbModel.readDfDb(databasePath)
                            showButton.enabled = true
                            statusText.text = statusInfo(model.status)
                        }
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

    Column {
        spacing: 10
        padding: 10
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 0

        Row {
        }
    }
}
