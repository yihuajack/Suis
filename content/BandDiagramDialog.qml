/**
 * Copyright (C) 2024 Yihua Liu
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
 */

import QtCharts
import QtQuick
import QtQuick.Controls

Dialog {
    id: bdChartDialog
    width: 800
    height: 600
    anchors.centerIn: parent
    modal: true
    standardButtons: Dialog.Ok

    contentItem: ChartView {
        id: plotChartView
        title: "Band Diagram"
        anchors.fill: parent
        margins.bottom: 50

        ValueAxis {
            id: axisX
            titleText: "x coordinate (nm)"
            min: 0
            max: cumsum(device.d) * 1e7
        }

        ValueAxis {
            id: axisY
            titleText: "E (eV)"
            min: Math.min.apply(null, device.VBM)
            max: 0
        }

        LineSeries {
            id: condBMSeries
            name: "CBM"
            axisX: axisX
            axisY: axisY

            Component.onCompleted: {
                condBMSeries.append(0, device.CBM[0])
                let x_pos = device.d[0] * 1e7
                condBMSeries.append(x_pos, device.CBM[0])
                for (let i = 1; i < device.col_size; i++) {
                    condBMSeries.append(x_pos, device.CBM[i])
                    x_pos += device.d[i] * 1e7
                    condBMSeries.append(x_pos, device.CBM[i])
                }
            }
        }

        LineSeries {
            id: valBMSeries
            name: "VBM"
            axisX: axisX
            axisY: axisY

            Component.onCompleted: {
                valBMSeries.append(0, device.VBM[0])
                let x_pos = device.d[0] * 1e7
                valBMSeries.append(x_pos, device.VBM[0])
                for (let i = 1; i < device.col_size; i++) {
                    valBMSeries.append(x_pos, device.VBM[i])
                    x_pos += device.d[i] * 1e7
                    valBMSeries.append(x_pos, device.VBM[i])
                }
            }
        }
    }

    function cumsum(list) {
        let sum = 0
        for (let i = 0; i < list.length; ++i) {
            sum += list[i]
        }
        return sum
    }
}