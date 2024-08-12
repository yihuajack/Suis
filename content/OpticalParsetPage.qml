/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

// import QtCore  // StandardPaths: QtLabsPlatform is deprecated since 6.4
import QtQuick
import QtQuick.Controls  // ProgressBar
import QtQuick.Dialogs  // native dialogs
import com.github.yihuajack.DbSysModel
// import SqlBrowser

OpticalParsetPageForm {
    // https://stackoverflow.com/questions/46627883/can-not-initialize-qml-property-to
    // https://stackoverflow.com/questions/37599362/is-it-not-possible-to-create-a-map-datatype-in-qml
    // property var optDbPaths: ({})
    // signal dbPathsChanged(var dbPaths)
    // It is not a good idea to initialize ListElement in a QML ListModel and put matDbModel in optLView delegates
    // because matDbModel will not persist data.

    // SqlBrowser {
    //     id: sqlBrowser
    // }

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
                        id: dbPathTextField
                        width: parent.width
                        enabled: model.checked && model.name !== "GCL"
                        placeholderText: "Enter the Database Path"
                        text: model.path
                        onEditingFinished: {  // method inherited from TextInput
                            // cannot merge this condition to importDb()!
                            // selectedFile will always mismatch this condition
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
                    id: dbImportButton
                    text: "Import"
                    enabled: model.checked
                    onClicked: {
                        if (model.name === "GCL") {
                            dbFileDialog.nameFilters = ["*.xlsx"]
                            dbLoginDialog.open();
                        } else {
                            if (model.name === "Solcore") {
                                dbFileDialog.nameFilters = ["*.txt", "*.ini"]
                            } else if (model.name === "Df") {
                                dbFileDialog.nameFilters = ["*.xlsx"]
                            }
                            dbFileDialog.open()
                        }
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
            }

            FileDialog {
                id: dbFileDialog
                title: qsTr("Select Device File")
                onAccepted: {
                    importDb(selectedFile)
                }
            }

            DbLoginDialog {
                id: dbLoginDialog
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
                                // https://forum.qt.io/topic/102523/reload-a-loader-on-button-click
                                // https://stackoverflow.com/questions/58716153/how-to-force-loader-to-reload-reset-or-delete-the-cache-of-preloaded-qml-page
                                // https://qml.guide/live-reloading-hot-reloading-qml/
                                nkChartLoader.active = false
                                nkChartLoader.source = "OpticalMaterialDialog.qml"
                                nkChartLoader.active = true
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
                if (dbPath !== model.path) {
                    statusText.text = "Importing optical materials from database"
                    // It's OK to property string databasePath: "" and databasePath = dbPath
                    // <Unknown File>: Can't assign to existing role 'path' of different type [Url -> String]
                    model.path = dbPath.toString()
                }
                // Even for the same path, we should allow re-importing because the same file may change.
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

    function statusInfo(status) {
        switch (status) {
            case 0:
                return "Database is successfully imported"
            case 1:
                return "Cannot find the path"
            case 2:
                return "Fail to load the n/k data"
            default:
                return "Invalid status"
        }
    }
}
