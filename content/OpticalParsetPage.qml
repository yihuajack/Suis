// Copyright (C) 2024 Yihua Liu
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 6.6
import QtQuick.Dialogs 6.6

OpticalParsetPageForm {
    id: opticalParsetPage

    state: "optInit"

    FolderDialog {
        id: optFolderDialog
        title: "Choose Optical Parameter Database Folder"
        onAccepted: {
            opticalParsetPage.state = "optParImported"
        }
    }
}
