pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import dev.ulduar.Constellar

Kirigami.Page {
    id: root

    required property var observer

    title: qsTr("Status")

    footer: Controls.ToolBar {
        visible: root.observer.zoneText.length > 0
        height: visible ? implicitHeight : 0

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: "mark-location-symbolic"
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
            }

            Controls.Label {
                text: root.observer.zoneText
                Layout.fillWidth: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: !root.observer.connected
            type: Kirigami.MessageType.Warning
            text: qsTr("constellard not reachable")
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: eventView

                anchors.fill: parent
                clip: true
                spacing: 0

                Controls.ScrollBar.vertical: Controls.ScrollBar {}

                model: root.observer.eventLog

                delegate: Delegates.RoundedItemDelegate {
                    id: row

                    width: ListView.view.width

                    required property string title
                    required property int activityState
                    required property double startTime
                    required property double stopTime
                    required property double durationMs

                    readonly property bool inProgress: activityState === ActivityModel.InProgress

                    text: row.title

                    icon.width: Kirigami.Units.iconSizes.smallMedium
                    icon.height: Kirigami.Units.iconSizes.smallMedium
                    icon.name: row.inProgress ? "media-playback-start-symbolic"
                             : row.activityState === ActivityModel.Success ? "checkmark-symbolic"
                             : "dialog-error-symbolic"
                    icon.color: row.inProgress ? Kirigami.Theme.highlightColor
                              : row.activityState === ActivityModel.Success ? Kirigami.Theme.positiveTextColor
                              : Kirigami.Theme.negativeTextColor

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.largeSpacing

                        Delegates.SubtitleContentItem {
                            itemDelegate: row
                            subtitle: row.inProgress ? ""
                                    : (row.activityState === ActivityModel.Success
                                            ? qsTr("cleared in %1") : qsTr("wiped after %1"))
                                          .arg(root.observer.durationText(row.durationMs))
                            Layout.fillWidth: true
                        }

                        Controls.Label {
                            text: {
                                root.observer.now;
                                return row.inProgress ? root.observer.elapsed(row.startTime)
                                                       : root.observer.relativeTime(row.stopTime)
                            }
                            opacity: 0.7
                            color: row.inProgress ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                        }
                    }
                }
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: eventView.count === 0
                icon.name: "fcitx-fullwidth-inactive"
                text: qsTr("No activities yet")
            }
        }
    }
}
