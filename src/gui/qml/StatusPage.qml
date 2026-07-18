import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: root

    required property var observer

    title: qsTr("Status")

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: !root.observer.connected
            type: Kirigami.MessageType.Warning
            text: qsTr("constellard not reachable")
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                level: 3
                text: qsTr("Activity: %1").arg(root.observer.activityText)
            }

            Kirigami.Heading {
                level: 3
                text: qsTr("Zone: %1").arg(root.observer.zoneText)
            }
        }

        Kirigami.CardsListView {
            id: eventView

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: root.observer.eventLog

            delegate: Kirigami.AbstractCard {
                id: card

                required property string message
                required property bool success

                contentItem: RowLayout {
                    Kirigami.Icon {
                        source: card.success ? "emblem-checked" : "emblem-error"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: card.message
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
