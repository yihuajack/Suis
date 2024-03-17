// Copyright (C) 2024 Yihua Liu
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 6.6
import QtQuick.Controls 6.6
import Suis 1.0

Rectangle {
    id: wizardRoot
    width: Constants.width
    height: Constants.height

    color: Constants.backgroundColor

    state: "initial"

    states: [
        State {
            name: "clicked"
            when: button.checked
        }
    ]
}
