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
        function simStart(scriptPath, devConf) {
            // Remember to add matlab/octave to path! -nosplash -nodesktop -r is deprecated; use -batch instead
            // simProcess.start("matlab", ["-batch", "try, cd('%1'), %2('%3'), catch me,
            //                  fprintf('%s / %s\n',me.identifier,me.message), end,
            //                  exit".arg(getParentDirectory(scriptPath)).arg(getBaseFileName(scriptPath)).arg(devConf)],
            //     Process.ReadOnly);
            // "flatpak", ["run", "org.octave.Octave", ...] if octave is installed via flatpak
            simProcess.start("octave", ["--no-gui", "--quiet", "--eval",
                "cd('%1'), %2('%3')".arg(getParentDirectory(scriptPath)).arg(getBaseFileName(scriptPath)).arg(devConf)],
                Process.ReadOnly);
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

            onButtonClicked: {
                let parentDirPath = getParentDirectory(wizardFlow.backendPath)
                simPage.path = parentDirPath + "/stats.csv";
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

            onCancelClicked: {
                wizardWindow.close()
            }

            onFinishClicked: {
                // let windowComponent = Qt.createComponent("qrc:/qt/qml/content/SimWindow.qml");
                // let newWindow = windowComponent.createObject(parent)
                wizardWindow.close();
                root.mainWindow.show();
                mainWindow.simStart(urlToPath(backendPath), urlToPath(devConfPath));
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

    function urlToPath(urlString) {
        // https://stackoverflow.com/questions/24927850/get-the-path-from-a-qml-url
        let s
        if (urlString.startsWith("file:///")) {
            let k = urlString.charAt(9) === ':' ? 8 : 7
            s = urlString.substring(k)
        } else {
            s = urlString
        }
        return decodeURIComponent(s);
    }

    function getParentDirectory(path) {
        let lastSlashIndex = path.lastIndexOf("/");
        if (lastSlashIndex > 0) {
            return path.substring(0, lastSlashIndex);
        }
        return path; // If no parent directory found, return the original path
    }

    function getBaseFileName(path) {
        // > The base name consists of all characters in the file up to (but not including) the first '.' character.
        // according to QString QFileInfo::baseName() const
        let lastSlashIndex = path.lastIndexOf("/");
        let fileName = (lastSlashIndex !== -1) ? path.substring(lastSlashIndex + 1) : path;

        let firstDotIndex = fileName.indexOf("."); // Get first occurrence of '.'
        return (firstDotIndex > 0) ? fileName.substring(0, firstDotIndex) : fileName;
    }
}
