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
        if (root.state === "initial") {
            root.state = "parSetOptical"
            stackView.push(opticalParsetPage)
        } else if (root.state === "parSetOptical") {
            root.state = "parSetElectrical"
            // opticalParsetPage.getOptMat("Ag")
            // stackView.push(electricalParsetPage, {elecDbPaths: opticalParsetPage.addDbPath()})
            stackView.push(electricalParsetPage)
        }
    }

    onBackClicked: {
        stackView.pop()
        if (root.state === "parSetOptical") {
            root.state = "initial"
        } else if (root.state === "parSetElectrical") {
            root.state = "parSetOptical"
        }
    }

    // StackView vs SwipeView
    // It seems that pushing and popping stacks is even simpler than to manually set visible properties
    // StackView provides transitions
    // State { name: ""\n PropertyChanges { target: welcomePage\n visible: true }
    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: WelcomePage {
            id: welcomePage
        }
    }

    OpticalParsetPage {
        id: opticalParsetPage
        visible: false
    }

    ElectricalParsetPage {
        id: electricalParsetPage
        visible: false
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
