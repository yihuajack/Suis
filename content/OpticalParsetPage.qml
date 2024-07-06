/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtCore  // StandardPaths: QtLabsPlatform is deprecated since 6.4
import QtQuick
import QtQuick.Controls  // ProgressBar
import QtQuick.Dialogs  // native dialogs
import DbSysModel

OpticalParsetPageForm {
    // https://stackoverflow.com/questions/46627883/can-not-initialize-qml-property-to
    // https://stackoverflow.com/questions/37599362/is-it-not-possible-to-create-a-map-datatype-in-qml
    // property var optDbPaths: ({})
    // signal dbPathsChanged(var dbPaths)
    // It is not a good idea to initialize ListElement in a QML ListModel and put matDbModel in optLView delegates
    // because matDbModel will not persist data.

    ListView {
        id: optLView
        // Warning: be careful when you write anchors.fill: parent
        // It will transparently cover the buttons and mislead you that buttons do not function!
        width: 800
        height: 600
        model: DbSysModel
        delegate: Item {
            width: parent.width
            height: 100

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
                        value: model.progress
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

                contentItem: ListView {
                    id: matLView
                    width: parent.width
                    height: parent.height - 20  // parent.height - header.height - 50
                    spacing: 10
                    // Do not use mode.db_model or others! See
                    // https://stackoverflow.com/questions/44747723/qt-qml-model-within-a-model-and-accesible-via-qml
                    model: db_model

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

            function importDb(dbPath) {
                statusText.text = "Importing optical materials from database"
                if (dbPath !== model.path) {
                    // It's OK to property string databasePath: "" and databasePath = dbPath
                    // <Unknown File>: Can't assign to existing role 'path' of different type [Url -> String]
                    model.path = dbPath.toString()
                }
                let status
                if (model.name === "Solcore") {
                    status = model.db_model.readSolcoreDb(model.path)
                } else if (model.name === "Df") {
                    status = model.db_model.readDfDb(model.path)
                }
                statusText.text = statusInfo(status)
                showButton.enabled = status === 0
            }
        }
    }

    // TypeError: Value is undefined and could not be converted to an object
    function getOptMat(mat_name) {
        for (let i = 0; i < optLView.model.count; i++) {  // count is different from size!
            // itemAtIndex(int index) is different from itemAt(real x, real y)!
            // Warning: The returned value of itemAtIndex() should not be stored since it can turn to null as soon as
            // control goes out of the calling scope, if the view releases that item.
            // https://stackoverflow.com/questions/65945283/qml-listview-acess-to-an-item-of-the-delegate
            let item = optLView.itemAtIndex(i)
            let matLView = item.matListDialog.contentItem.children.find(child => child.id === "matLView")
            for (let j = 0; j < matLView.model.count; j++) {
                console.log("j=" + j)
                let mat_item = matLView.model.get(j)
                if (mat_item.name === mat_name) {
                    console.log("item.n_wl[0]=" + item.n_wl[0])
                    break;
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
