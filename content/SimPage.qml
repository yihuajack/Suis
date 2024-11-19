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
            text: "Upload"
            onClicked: {
                let uploaded = SqlTreeModel.upload()
                if (!uploaded) {
                    term.text += ("\nFailed to upload!")
                } else {
                    term.text += ("\nSuccessfully uploaded!")
                }
            }
        }
    }
}
