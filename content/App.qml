/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

// Since Qt 6.0, Qt and QtQuick, QtQuick.Controls have the same version number.
import QtQuick
import QtQuick.Controls
import QtQuick.VirtualKeyboard
import Suis
import Process

QtObject {
    id: root

    property var wizardWindow: Window {
        width: Screen.desktopAvailableWidth
        height: Screen.desktopAvailableHeight

        visible: false
        title: qsTr("Suis Parameter Wizard")

        WizardFlow {
            id: wizardFlow
            onCancelClicked: wizardWindow.close()
            onFinishClicked: {
                // let windowComponent = Qt.createComponent("qrc:/qt/qml/content/SimWindow.qml");
                // let newWindow = windowComponent.createObject(parent)
                wizardWindow.close()
                root.mainWindow.show()

                // "-nosplash", "-nodesktop", "-r"
                root.mainWindow.simProcess.start("matlab", ["-batch", "\"run('E:/Documents/GitHub/ddmodel-octave/demo_eco_pin.m')\""], Process.ReadOnly)
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

        SimPage {
            id: simPage
        }

        Process {
            id: simProcess

            property string output: ""

            onReadyReadStandardOutput: {
                output = simProcess.readAll()
                term.text += output
            }
        }
    }

    property var splashWindow: Splash {
        visible: true
        onTimeout: wizardWindow.visible = true
    }
}
