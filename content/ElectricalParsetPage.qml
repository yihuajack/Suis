import QtQuick
import QtQuick.Dialogs 6.6
import QtCore 6.6  // StandardPaths: QtLabsPlatform is deprecated since 6.4

ElectricalParsetPageForm {
    id: electricalParsetPage

    state: "elecInit"

    FolderDialog {
        id: elecFolderDialog
        title: "Choose Electrical Parameter Database Folder"
        currentFolder: StandardPaths.standardLocations(StandardPaths.AppConfigLocation)[0]
        onAccepted: {
            electricalParsetPage.state = "elecParImported"
        }
    }
}
