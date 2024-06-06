/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls
import Suis

Rectangle {
    id: root
    anchors.fill: parent

    signal backClicked
    signal nextClicked
    signal finishClicked
    signal cancelClicked

    color: Constants.backgroundColor

    Rectangle {
        id: bottomBar
        width: parent.width
        height: 100
        color: Constants.barColor
        anchors.bottom: parent.bottom
        // anchors.bottomMargin: Constants.defaultMargin

        Button {
            id: backButton
            text: qsTr("< Back")
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 0
            anchors.right: parent.right
            anchors.rightMargin: Constants.defaultMargin + 200
            checkable: true

            Connections {
                target: backButton
                onClicked: backClicked()
            }
        }

        Button {
            id: nextButton
            text: qsTr("Next >")
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 0
            anchors.right: parent.right
            anchors.rightMargin: Constants.defaultMargin + 100
            checkable: true
            visible: root.state !== "parSetElectrical"

            Connections {
                target: nextButton
                onClicked: nextClicked()
            }
        }

        Button {
            id: finishButton
            text: qsTr("Finish")
            visible: root.state === "parSetElectrical"
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 0
            anchors.right: parent.right
            anchors.rightMargin: Constants.defaultMargin + 100
            checkable: true

            Connections {
                target: finishButton
                onClicked: finishClicked()
            }
        }

        Button {
            id: cancelButton
            text: qsTr("Cancel")
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 0
            anchors.right: parent.right
            anchors.rightMargin: Constants.defaultMargin
            checkable: true

            Connections {
                target: cancelButton
                onClicked: cancelClicked()
            }
        }
    }
}
