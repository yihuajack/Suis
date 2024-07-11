/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtCharts
import QtQuick
import QtQuick.Controls

Dialog {
    id: ratChartDialog
    width: 800
    height: 600
    anchors.centerIn: parent
    modal: true
    standardButtons: Dialog.Ok

    contentItem: ChartView {
        id: plotChartView
        title: "Plot of R, A, and T vs. Wavelengths"
        anchors.fill: parent

        ValueAxis {
            id: axisX
            titleText: "Wavelength (nm)"
            min: Math.min.apply(null, model.wavelength) * 1e9
            max: Math.max.apply(null, model.wavelength) * 1e9
        }

        ValueAxis {
            id: axisY
            min: 0
            max: 1
        }

        LineSeries {
            id: rSeries
            name: "R"
            axisX: axisX
            axisY: axisY

            Component.onCompleted: {
                for (let i = 0; i < model.wavelength.length; i++) {
                    nSeries.append(model.wavelength[i] * 1e9, model.R[i]);
                }
            }
        }

        LineSeries {
            id: aSeries
            name: "A"
            axisX: axisX
            axisY: axisY

            Component.onCompleted: {
                for (let i = 0; i < model.wavelength.length; i++) {
                    nSeries.append(model.wavelength[i] * 1e9, model.A[i]);
                }
            }
        }

        LineSeries {
            id: tSeries
            name: "T"
            axisX: axisX
            axisY: axisY

            Component.onCompleted: {
                for (let i = 0; i < model.wavelength.length; i++) {
                    nSeries.append(model.wavelength[i] * 1e9, model.T[i]);
                }
            }
        }
    }
}