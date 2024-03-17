// Copyright (C) 2024 Yihua Liu
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 6.6
import Suis
import QtQuick.VirtualKeyboard 6.6

Window {
    id: mainWindow
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight

    visible: true
    title: qsTr("Suis")

    WizardFlow {
        onCancelClicked: mainWindow.close()
    }

    InputPanel {
        id: inputPanel
        property bool showKeyboard :  active
        y: showKeyboard ? parent.height - height : parent.height
        Behavior on y {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
        anchors.leftMargin: Constants.width / 10
        anchors.rightMargin: Constants.width / 10
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
