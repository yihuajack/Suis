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
            titleText: "Wavelength (nm)"
            min: Math.min(Math.min.apply(null, model.n_wl), Math.min.apply(null, model.k_wl)) * 1e9
            max: Math.max(Math.max.apply(null, model.n_wl), Math.max.apply(null, model.k_wl)) * 1e9
        }

        ValueAxis {
            id: axisY
            min: 0
            max: Math.max(Math.max.apply(null, model.n_data), Math.max.apply(null, model.k_data)) * 1.2
        }

        LineSeries {
            id: nSeries
            name: "n"
            axisX: axisX
            axisY: axisY

            Component.onCompleted: {
                for (let i = 0; i < model.n_wl.length; i++) {
                    nSeries.append(model.n_wl[i] * 1e9, model.n_data[i]);  // Wavelength unit: m -> nm
                }
            }
        }

        LineSeries {
            id: kSeries
            name: "k"
            axisX: axisX
            axisY: axisY

            Component.onCompleted: {
                for (let i = 0; i < model.k_wl.length; i++) {
                    kSeries.append(model.k_wl[i] * 1e9, model.k_data[i]);
                }
            }
        }
    }
}