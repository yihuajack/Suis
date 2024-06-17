/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import Suis

WizardFlowForm {
    id: root
    width: Constants.width
    height: Constants.height

    state: "initial"

    onNextClicked: {
        if (root.state === "initial")
            root.state = "parSetOptical"
        else if (root.state === "parSetOptical")
            root.state = "parSetElectrical"
    }

    WelcomePage {
        id: welcomePage
        visible: root.state === "initial"
        anchors.fill: parent
    }

    OpticalParsetPage {
        id: opticalParsetPage
        visible: root.state === "parSetOptical"
        anchors.fill: parent
    }

    ElectricalParsetPage {
        id: electricalParsetPage
        visible: root.state === "parSetElectrical"
        anchors.fill: parent
    }

    Component.onCompleted: {
        opticalParsetPage.elecDbCandChanged.connect(electricalParsetPage.receiveElecDbCand)
    }

    states: [
        State {
            name: "initial"
        },
        State {
            name: "parSetOptical"
        },
        State {
            name: "parSetElectrical"
        }
    ]
}
