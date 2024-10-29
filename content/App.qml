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

    property var mainWindow: ApplicationWindow {
        function simStart() {
            // Remember to add matlab to path!
            // simProcess.start("matlab", ["-nosplash", "-nodesktop", "-r", "run('E:/Documents/GitHub/ddmodel-octave/demo_ms_pin.m')"], Process.ReadOnly);
            // simProcess.start("matlab", ["-batch", "run('E:/Documents/GitHub/ddmodel-octave/demo_ms_pin.m')"], Process.ReadOnly);
            simProcess.start("octave", ["--no-gui", "--quiet", "'E:/Documents/GitHub/ddmodel-octave/demo_ms_pin.m'"], Process.ReadOnly);
        }

        height: 480
        title: qsTr("Suis")
        visible: false
        width: 640

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
                output = simProcess.readAllStandardOutput();
                term.text += output;
            }
        }
    }
    property var splashWindow: Splash {
        visible: true

        onTimeout: wizardWindow.visibility = Window.Windowed
    }
    property var wizardWindow: Window {
        height: Constants.height
        title: qsTr("Suis Parameter Wizard")
        visibility: Window.Hidden
        // screen: Qt.application.screens[0]
        width: Constants.width

        WizardFlow {
            id: wizardFlow

            onCancelClicked: wizardWindow.close()
            onFinishClicked: {
                // let windowComponent = Qt.createComponent("qrc:/qt/qml/content/SimWindow.qml");
                // let newWindow = windowComponent.createObject(parent)
                wizardWindow.close();
                root.mainWindow.show();
                mainWindow.simStart();
            }
        }
        InputPanel {
            id: inputPanel

            property bool showKeyboard: active

            anchors.left: parent.left
            anchors.leftMargin: Constants.width / 10
            anchors.right: parent.right
            anchors.rightMargin: Constants.width / 10
            y: showKeyboard ? parent.height - height : parent.height

            Behavior on y {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
