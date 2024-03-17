/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick 6.6
import QtQuick.Controls 6.6
import Suis 1.0

Item {
    id: root
    width: Constants.width
    height: Constants.height - 100
    anchors.top: parent.top

    Button {
        id: importOptButton
        text: qsTr("Import Optical Parameters")
        y: 300
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 0
        checkable: true

        Connections {
            target: importOptButton
            onClicked: optFolderDialog.open()
        }
    }

    Button {
        id: plotOptButton
        text: qsTr("Plot Optical Parameters")
        visible: opticalParsetPage.state === "optParImported"
        y: 600
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 0
        checkable: true

        Connections {
            target: plotOptButton
            onClicked: {
                raImage.visible = true
            }
        }
    }

    Image {
        id: raImage
        source: "file:///E:/Documents/Master/ECO/GCL/slide_materials/feb_26/RA_device2.png"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Constants.defaultMargin + 150
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 0
        fillMode: Image.PreserveAspectFit
        visible: false
    }

    states: [
        State {
            name: "optInit"
        },
        State {
            name: "optParImported"
        }
    ]
}
