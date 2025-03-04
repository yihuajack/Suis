/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import Suis

Item {
    id: root
    width: Constants.width
    height: Constants.height - 100

    property string backendPath: ""

    Text {
        id: label
        text: qsTr("Parameter Wizard")
        font.family: Constants.font.family
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 0
        font.pointSize: 24
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter

        SequentialAnimation {
            id: animation

            ColorAnimation {
                id: colorAnimation1
                target: label
                property: "color"
                to: "#2294c6"
                from: Constants.backgroundColor
            }

            ColorAnimation {
                id: colorAnimation2
                target: label
                property: "color"
                to: Constants.backgroundColor
                from: "#2294c6"
            }
        }
    }

    Row {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        spacing: 10

        TextField {
            id: backendPathInput
            width: 400
            height: 50
            text: backendPath
            color: "black"
            // font.pixelSize: 16
            placeholderText: "Enter backend path"
            background: Rectangle {
                color: "white"
                border.color: "gray"
                radius: 5
            }
            onTextChanged: {
                backendPath = text
            }
        }

        Button {
            text: "Select"
            height: 50
            onClicked: {
                backendFileDialog.open()
            }
        }

        FileDialog {
            id: backendFileDialog
            title: qsTr("Select Backend Path")
            onAccepted: {
                backendPathInput.text = selectedFile
                backendPath = selectedFile
            }
        }
    }
}
