// Copyright (C) 2024 Yihua Liu
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Dialogs
import QtCore  // StandardPaths: QtLabsPlatform is deprecated since 6.4
import MaterialSystemModel

OpticalParsetPageForm {
    id: opticalParsetPage

    state: "optInit"

    FolderDialog {
        id: optFolderDialog
        title: "Choose Optical Parameter Database Folder"
        currentFolder: StandardPaths.standardLocations(StandardPaths.AppConfigLocation)[0]
        onAccepted: {
            opticalParsetPage.state = "optParImported"
            dbPathText = currentFolder
        }
    }

    MaterialSystemModel {
        id: matSysModel
    }

    ListView {
        model: matSysModel

        delegate: Item {
            width: parent.width
            height: 50
            RowLayout {
                anchors.fill: parent

                Text {
                    text: matSysModel.db_name
                }

                TextField {
                    id: dbPathText
                    Layout.fillWidth: true
                    placeholderText: "Enter database path"
                }

                Button {
                    text: "Select"
                    onClicked: optFolderDialog.open()
                }
            }
        }
    }
}
