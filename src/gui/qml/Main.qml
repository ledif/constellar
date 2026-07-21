import QtQuick
import org.kde.kirigami as Kirigami

import dev.ulduar.Constellar

Kirigami.ApplicationWindow {
    id: root

    title: qsTr("Constellar")

    width: 480
    height: 360

    ObserverController {
        id: observer
    }

    globalDrawer: Kirigami.GlobalDrawer {
        title: root.title
        titleIcon: "applications-games"

        actions: [
            Kirigami.Action {
                text: qsTr("Refresh")
                icon.name: "view-refresh"
                onTriggered: observer.refresh()
            }
        ]
    }

    pageStack.initialPage: StatusPage {
        observer: observer
    }
}
