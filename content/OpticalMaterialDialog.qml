/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtCharts
import QtQuick  // Component; non-existent attached object
import QtQuick.Controls  // non-native Dialog

Dialog {
    id: nkChartDialog
    width: 800
    height: 600
    anchors.centerIn: parent
    modal: true  // modality: Qt.WindowModel
    standardButtons: Dialog.Ok

    contentItem: ChartView {
        id: plotChartView
        title: "Plot of n and k vs. Wavelengths"
        // QML Row: Cannot specify left, right, horizontalCenter, fill or centerIn anchors for items inside Row.
        // Row will not function.
        anchors.fill: parent

        ValueAxis {
            id: axisX
            min: 0
            max: model.n_wl.length - 1
            titleText: "Wavelength"
        }

        ValueAxis {
            id: axisY
            min: Math.min.apply(null, model.n_wl) - 1
            max: Math.max.apply(null, model.n_wl) + 1
            titleText: "Fraction"
        }

        LineSeries {
            id: nkseries
            name: "n and k"
            axisX: axisX
            axisY: axisY

            Component.onCompleted: {
                for (let i = 0; i < model.n_wl.length; i++) {
                    nkseries.append(i, model.n_wl[i]);
                }
            }
        }
    }
}