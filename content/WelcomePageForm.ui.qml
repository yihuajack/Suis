

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
    height: Constants.height

    signal startClicked
    signal exitClicked

    Button {
        id: startButton
        text: qsTr("Start")
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenterOffset: -50
        checkable: true
        anchors.horizontalCenter: parent.horizontalCenter

        Connections {
            target: startButton
            onClicked: startClicked()
        }
    }

    Button {
        id: exitButton
        text: qsTr("Exit")
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenterOffset: 50
        checkable: true
        anchors.horizontalCenter: parent.horizontalCenter

        Connections {
            target: button
            onClicked: exitClicked()
        }
    }

    Text {
        id: label
        y: 213
        text: qsTr("Parameter Wizard")
        anchors.top: button.bottom
        font.family: Constants.font.family
        anchors.topMargin: -418
        font.pointSize: 24
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter

        SequentialAnimation {
            id: animation

            ColorAnimation {
                id: colorAnimation1
                target: rectangle
                property: "color"
                to: "#2294c6"
                from: Constants.backgroundColor
            }

            ColorAnimation {
                id: colorAnimation2
                target: rectangle
                property: "color"
                to: Constants.backgroundColor
                from: "#2294c6"
            }
        }
    }
}
