/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

// Since Qt 6.0, Qt and QtQuick, QtQuick.Controls have the same version number.
import QtQuick
import QtQuick.Controls
import QtQuick.VirtualKeyboard
import Suis
import content

QtObject {
    id: root

    property var wizardWindow: Window {
        // screen: Qt.application.screens[0]
        width: Constants.width
        height: Constants.height
        visibility: Window.Hidden
        title: qsTr("Suis Parameter Wizard")

        WizardFlow {
            id: wizardFlow
            onCancelClicked: wizardWindow.close()
            onFinishClicked: {
                // let windowComponent = Qt.createComponent("qrc:/qt/qml/content/SimWindow.qml");
                // let newWindow = windowComponent.createObject(parent)
                wizardWindow.close()
                root.mainWindow.show()
                mainWindow.simStart()
            }
        }

        InputPanel {
            id: inputPanel
            property bool showKeyboard : active
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

    property var mainWindow: ApplicationWindow {
        visible: false
        width: 640
        height: 480
        title: qsTr("Suis")

        function simStart() {
            // "-nosplash", "-nodesktop", "-r"
            // simProcess.start("matlab", ["-batch", "\"run('E:/Documents/GitHub/ddmodel-octave/demo_ms_pin.m')\""], Process.ReadOnly)
            simProcess.start("matlab", Process.ReadOnly)
        }

        SimPage {
            id: simPage

            ScrollView {
                id: view
                anchors.fill: parent

                TextArea {
                    id: term
                    textFormat: TextArea.RichText
                }
            }
        }

        Process {
            id: simProcess

            property string output: ""

            onReadyReadStandardOutput: {
                output = simProcess.readAllStandardOutput()
                term.text += output
            }
        }
    }

    property var splashWindow: Splash {
        visible: true
        onTimeout: wizardWindow.visibility = Window.Windowed
    }
}
