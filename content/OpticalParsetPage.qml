// Copyright (C) 2024 Yihua Liu
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Dialogs 6.6
import QtCore 6.6  // StandardPaths: QtLabsPlatform is deprecated since 6.4

OpticalParsetPageForm {
    id: opticalParsetPage

    state: "optInit"

    FolderDialog {
        id: optFolderDialog
        title: "Choose Optical Parameter Database Folder"
        currentFolder: StandardPaths.standardLocations(StandardPaths.AppConfigLocation)[0]
        onAccepted: {
            opticalParsetPage.state = "optParImported"
        }
    }
}
