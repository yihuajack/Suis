// Copyright (C) 2024 Yihua Liu
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 6.6
import Suis
import QtQuick.VirtualKeyboard 6.6
import Process 1.0

ApplicationWindow {
    id: simWindow
    visible: false
    width: 640
    height: 480
    title: qsTr("Hello World")

    Process {
        id: simProcess

        property string output: ""

        onStarted: print("Started")
        onFinished: print("Closed")

        onReadyReadStandardOutput: {
            output = simProcess.readAll()
            term.text += output
        }
    }
    Component.onCompleted: {
        simProcess.start("bash")
    }

    // use py.close() or py.kill(0 or py.terminate() to kill the process
    Component.onDestruction: simProcess.close()

    Page {
        anchors.fill: parent

        ScrollView {
            id: view
            anchors.fill: parent

            TextArea {
                id: term
                textFormat: TextArea.RichText
            }
        }

        footer: RowLayout {
            Layout.fillWidth: true

            TextField {
                id: input
                Layout.fillWidth: true
            }

            Button {
                text: "Send!"
                property string command: input.text.trim() + "\n"
                onClicked: {
                    simProcess.write(command)
                    term.text += ("<br /><b>" + command + " : </b>")
                }
            }
        }
    }
}
