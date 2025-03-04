/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.VirtualKeyboard
import content

Page {
    id: root
    anchors.fill: parent

    property string path: "init"
    signal buttonClicked(path: string)

    footer: RowLayout {
        Layout.fillWidth: true

        TextField {
            id: input
            Layout.fillWidth: true
        }

        Button {
            text: "Send"
            property string command: input.text.trim() + "\n"
            onClicked: {
                simProcess.write(command)
                term.text += ("\n" + command + " :")
            }
        }

        Button {
            id: uploadButton
            text: "Upload"
            onClicked: {
                buttonClicked(path)
                let uploaded = SqlTreeModel.upload(path)
                if (!uploaded) {
                    term.text += ("\nFailed to upload!")
                } else {
                    term.text += ("\nSuccessfully uploaded!")
                }
            }
        }
    }
}
