/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls  // Dialog
import QtQuick.Dialogs
import QtCore  // StandardPaths: QtLabsPlatform is deprecated since 6.4
import Qt.labs.folderlistmodel
import MaterialDbModel

OpticalParsetPageForm {
    id: opticalParsetPage

    property string databasePath: ""
    property var matList: []

    Column {
        spacing: 10
        padding: 10
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 0

        Row {
            spacing: 10

            ComboBox {
                id: dbTypeCombo
                width: 150
                model: ["Solcore", "Suis"]
                onCurrentIndexChanged: {
                    importButton.enabled = currentIndex !== -1
                }
            }

            TextField {
                id: pathTextField
                width: 400
                placeholderText: "Database Path"
                text: databasePath
                onTextChanged: {
                    databasePath = text
                }
            }

            Button {
                id: importButton
                text: "Import"
                enabled: false
                onClicked: {
                    if (dbTypeCombo.currentIndex === 0) {
                        folderDialog.open()
                    } else if (dbTypeCombo.currentIndex === 1) {
                        fileDialog.open()
                    }
                }
            }

            Button {
                id: showButton
                text: "Show"
                onClicked: {
                    if (databasePath !== "") {
                        if (dbTypeCombo.currentIndex === 0) {
                            matList = readSolcoreDb(databasePath)
                        } else if (dbTypeCombo.currentIndex === 1) {
                            matList = readDfDb(databasePath)
                        }
                        fileListDialog.open()
                    }
                }
            }
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Select Database Folder"
        onAccepted: {
            databasePath = selectedFolder
            pathTextField.text = selectedFolder
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select Database File"
        nameFilters: ["*.xlsx"]
        onAccepted: {
            databasePath = selectedFile
            pathTextField.text = selectedFile
        }
    }

    Dialog {
        id: fileListDialog
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

    FolderListModel {
        id: folderListModel
    }
}
