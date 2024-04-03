/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls 6.6
import Suis 1.0

Item {
    id: root
    width: Constants.width
    height: Constants.height - 100
    anchors.top: parent.top

    Button {
        id: importElecButton
        text: qsTr("Import Electrical Parameters")
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 0
        checkable: true

        Connections {
            target: importElecButton
            onClicked: elecFolderDialog.open()
        }
    }

    states: [
        State {
            name: "elecInit"
        },
        State {
            name: "elecParImported"
        }
    ]
}
