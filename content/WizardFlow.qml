// Copyright (C) 2024 Yihua Liu
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 6.6
import QtQuick.Controls 6.6
import Suis 1.0

WizardFlowForm {
    id: wizardFlow
    width: Constants.width
    height: Constants.height

    state: "initial"

    onNextClicked: {
        if (wizardFlow.state === "initial")
            wizardFlow.state = "parSetOptical"
        else if (wizardFlow.state === "parSetOptical")
            wizardFlow.state = "parSetElectrical"
    }

    WelcomePage {
        id: welcomePage
        visible: wizardFlow.state === "initial"
        anchors.fill: parent
    }

    OpticalParsetPage {
        id: opticalParsetPage
        visible: wizardFlow.state === "parSetOptical"
        anchors.fill: parent
    }

    ElectricalParsetPage {
        id: electricalParsetPage
        visible: wizardFlow.state === "parSetElectrical"
        anchors.fill: parent
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
