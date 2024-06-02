/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Dialogs
import QtCore  // StandardPaths: QtLabsPlatform is deprecated since 6.4
import DbModel
import MaterialSystemModel

OpticalParsetPageForm {
    id: opticalParsetPage

    state: "optInit"

    DbModel {
        id: dbModel
    }

    FolderDialog {
        id: optFolderDialog
        title: "Choose the Optical Parameter Database Folder"
        currentFolder: StandardPaths.standardLocations(StandardPaths.AppConfigLocation)[0]  // Qt.resolvedUrl("./")
        onAccepted: {
            opticalParsetPage.state = "optParImported"
            dbModel.dbPath = selectedFolder
        }
    }

    MaterialSystemModel {
        id: matSysModel
    }

    Dialog {
        id: matListDialog
        width: 400
        height: 300
        title: "Imported Materials"

        ListView {
            anchors.fill: parent
            model: dbModel

            delegate: Item {
                width: parent.width
                height: 50

                Text {
                    text: model.dbName
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    ListView {
        id: dbListView
        model: matSysModel

        delegate: Item {
            width: parent.width
            height: 50
            Grid {
                columns: 3
                rows: 2  // by default
                spacing: 10
                Text {
                    id: dbNameText
                    text: matSysModel.db_name
                    anchors {
                        left: parent.left
                        leftMargin: 10
                        verticalCenter: parent.verticalCenter
                    }
                }

                TextField {  // not readOnly
                    id: dbPathText
                    anchors {
                        left: dbNameText.right
                        leftMargin: 10
                        verticalCenter: parent.verticalCenter
                    }
                    Layout.fillWidth: true
                    placeholderText: "Enter database path"
                    text: selectedFolderPath
                }

                Button {
                    text: "Select"
                    width: 100
                    height: 30
                    anchors {
                        right: parent.right
                        rightMargin: 10
                        verticalCenter: parent.verticalCenter
                    }
                    onClicked: optFolderDialog.open()
                }

                Item {}

                Text {
                    anchors {
                        top: rowLayout.bottom + 10
                        left: rowLayout.dbPathText.left
                        verticalCenter: parent.verticalCenter
                    }
                }

                Button {
                    text: "Show"
                    width: 100
                    height: 30
                    anchors {
                        right: parent.right
                        rightMargin: 10
                        verticalCenter: parent.verticalCenter
                    }
                    onClicked: matListDialog.open()
                }
            }
        }
    }
}
