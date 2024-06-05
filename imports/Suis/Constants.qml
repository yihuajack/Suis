pragma Singleton
import QtQuick
import QtQuick.Studio.Application

QtObject {
    function getMinWidth() {
        let minWidth = Screen.width
        const screens = Qt.application.screens
        // Note that screen[i].desktopAvailableWidth is always the total width of multiple monitor screens if they are
        // set as extended horizontally.
        for (let i = 0; i < screens.length; i++) {
            if (screens[i].width < minWidth) {
                minWidth = screens[i].width
            }
        }
        return minWidth
    }

    function getMinHeight() {
        let minHeight = Screen.height
        let maxHeight = Screen.height
        const screens = Qt.application.screens
        // Note that screen[i].desktopAvailableHeight is always the maximum height of multiple monitor screens if they
        // are set as extended horizontally.
        for (let i = 0; i < screens.length; i++) {
            if (screens[i].height < minHeight) {
                minHeight = screens[i].height
            }
            if (screens[i].height > maxHeight) {
                maxHeight = screens[i].height
            }
        }
        return minHeight - maxHeight + Screen.desktopAvailableHeight  // excluding window manager reserved areas
    }

    readonly property int width: 1024
    readonly property int height: 768

    property string relativeFontDirectory: "fonts"

    /* Edit this comment to add your custom font */
    readonly property font font: Qt.font({
                                             family: Qt.application.font.family,
                                             pixelSize: Qt.application.font.pixelSize
                                         })
    readonly property font largeFont: Qt.font({
                                                  family: Qt.application.font.family,
                                                  pixelSize: Qt.application.font.pixelSize * 1.6
                                              })

    readonly property color backgroundColor: "#c2c2c2"
    readonly property color barColor: "Grey"
    readonly property int defaultMargin: 20

    property StudioApplication application: StudioApplication {
        fontPath: Qt.resolvedUrl("../../content/" + relativeFontDirectory)
    }
}
