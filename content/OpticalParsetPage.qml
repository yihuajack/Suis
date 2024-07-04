/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtCore  // StandardPaths: QtLabsPlatform is deprecated since 6.4
import QtQuick
import QtQuick.Controls  // ProgressBar
import QtQuick.Dialogs  // native dialogs
import content
import MaterialDbModel

OpticalParsetPageForm {
    // https://stackoverflow.com/questions/46627883/can-not-initialize-qml-property-to
    // https://stackoverflow.com/questions/37599362/is-it-not-possible-to-create-a-map-datatype-in-qml
    // property var optDbPaths: ({})
    // signal dbPathsChanged(var dbPaths)

    ListModel {
        id: dbSysModel
        ListElement {
            name: "Solcore"
            checked: false
            path: ""
        }

        ListElement {
            name: "Df"
            checked: false
            path: ""
        }
    }

    ListView {
        id: optLView
        // Warning: be careful when you write anchors.fill: parent
        // It will transparently cover the buttons and mislead you that buttons do not function!
        width: 800
        height: 600
        model: dbSysModel
        delegate: Item {
            width: parent.width
            height: 100

            MaterialDbModel {
                id: matDbModel
            }

            Row {
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter

                CheckBox {
                    width: 100
                    text: model.name
                    onCheckedChanged: {
                        model.checked = checked
                    }
                }

                Column {
                    width: parent.width * 0.6
                    spacing: 10

                    TextField {
                        id: pathTextField
                        width: parent.width
                        enabled: model.checked
                        placeholderText: "Enter the Database Path"
                        text: model.path
                        onEditingFinished: {  // method inherited from TextInput
                            if (text.length > 0) {
                                importDb(text)
                            }
                        }
                    }

                    ProgressBar {
                        id: importProgressBar
                        width: parent.width
                        to: 1.0
                        value: matDbModel.progress
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
                    enabled: false
                    onClicked: {
                        matListDialog.open()
                    }
                }

                FileDialog {
                    id: fileDialog
                    title: qsTr("Select Database File")
                    onAccepted: {
                        importDb(selectedFile)
                    }
                }
            }

            // Inherit from Popup
            Dialog {
                id: matListDialog
                title: "Imported Optical Materials"
                width: parent.width * 0.8
                height: parent.width * 0.6
                anchors.centerIn: parent
                modal: true  // modality: Qt.WindowModel
                standardButtons: Dialog.Ok

                contentItem: Column {
                    spacing: 10
                    padding: 10

                    ListView {
                        width: parent.width
                        height: parent.height - 20  // parent.height - header.height - 50
                        spacing: 10
                        model: matDbModel  // Qt.labs.folderlistmodel

                        delegate: Row {
                            spacing: 10

                            Text {
                                text: model.name
                                width: parent.width * 0.7
                            }

                            Button {
                                text: "Plot"
                                width: 100
                                onClicked: {
                                    nkChartLoader.source = "OpticalMaterialDialog.qml"
                                }
                            }

                            // https://doc.qt.io/qt-6/qtquick-performance.html
                            Loader {
                                id: nkChartLoader
                                asynchronous: true

                                onLoaded: item.open()
                            }
                        }
                    }
                }
            }

            function importDb(dbPath) {
                statusText.text = "Importing optical materials from database"
                if (dbPath !== model.path) {
                    // It's OK to property string databasePath: "" and databasePath = dbPath
                    // <Unknown File>: Can't assign to existing role 'path' of different type [Url -> String]
                    model.path = dbPath.toString()
                }
                let status
                if (model.name === "Solcore") {
                    status = matDbModel.readSolcoreDb(model.path)
                } else if (model.name === "Df") {
                    status = matDbModel.readDfDb(model.path)
                }
                statusText.text = statusInfo(status)
                showButton.enabled = status === 0
            }
        }
    }

    // TypeError: Value is undefined and could not be converted to an object
    function addDbPath() {
        let optDbPaths = {}
        for (let i = 0; i < optLView.model.count; i++) {
            let item = optLView.model.get(i)
            if (item.checked && item.name !== item.path) {
                optDbPaths[item.name] = item.path
            }
        }
        return optDbPaths
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
