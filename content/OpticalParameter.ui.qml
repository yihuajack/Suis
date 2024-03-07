

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    width: 1280
    height: 720

    Button {
        id: button
        x: 886
        y: 638
        text: qsTr("< Back")
    }

    Button {
        id: button1
        x: 997
        y: 638
        text: qsTr("Next >")
    }

    Button {
        id: button2
        x: 1115
        y: 638
        text: qsTr("Cancel")
    }

    Button {
        id: button3
        x: 99
        y: 93
        width: 461
        height: 438
        text: qsTr("Import Optical Data")
        flat: false
        font.kerning: true
        font.pointSize: 18
    }

    Button {
        id: button4
        x: 630
        y: 93
        width: 461
        height: 438
        text: qsTr("Import Electrical Data")
        font.pointSize: 18
        font.kerning: true
        flat: false
    }
}
