#include "PresencePublisher.h"

#include "DiscordIpcClient.h"

PresencePublisher::PresencePublisher(DiscordIpcClient &client, QObject *parent)
    : QObject(parent), m_client(client) {}

QJsonObject PresencePublisher::encounterActivity(const QString &encounterName,
                                                 const QString &difficulty,
                                                 const QDateTime &startTime) {
    QJsonObject activity;
    activity["details"] = QStringLiteral("%1 %2").arg(difficulty, encounterName);
    activity["state"] = QStringLiteral("Raid Encounter");
    QJsonObject timestamps;
    timestamps["start"] = startTime.toUTC().toSecsSinceEpoch();
    activity["timestamps"] = timestamps;
    return activity;
}

QJsonObject PresencePublisher::dungeonActivity(int keystoneLevel, const QDateTime &startTime) {
    QJsonObject activity;
    activity["details"] = QStringLiteral("Mythic+ Key +%1").arg(keystoneLevel);
    activity["state"] = QStringLiteral("In a dungeon");
    QJsonObject timestamps;
    timestamps["start"] = startTime.toUTC().toSecsSinceEpoch();
    activity["timestamps"] = timestamps;
    return activity;
}

QJsonObject PresencePublisher::idleActivity() {
    QJsonObject activity;
    // No realm/zone/character here yet -- that's the addon-channel hybrid
    // (ADR-011), not built. Just says wowcapd sees WoW running.
    activity["details"] = QStringLiteral("In World of Warcraft");
    return activity;
}

void PresencePublisher::onEncounterDetected(int /*encounterId*/, const QString &encounterName,
                                            const QString &difficulty, const QString &startTime) {
    m_client.setActivity(encounterActivity(encounterName, difficulty,
                                           QDateTime::fromString(startTime, Qt::ISODateWithMs)));
}

void PresencePublisher::onEncounterEnded(int /*encounterId*/, const QString & /*encounterName*/,
                                         bool /*success*/, const QString & /*stopTime*/) {
    m_client.setActivity(idleActivity());
}

void PresencePublisher::onDungeonDetected(int /*zoneId*/, int /*mapId*/, int keystoneLevel,
                                          const QString &startTime) {
    m_client.setActivity(
        dungeonActivity(keystoneLevel, QDateTime::fromString(startTime, Qt::ISODateWithMs)));
}

void PresencePublisher::onDungeonEnded(int /*mapId*/, int /*keystoneLevel*/, bool /*success*/,
                                       int /*durationMs*/, const QString & /*stopTime*/) {
    m_client.setActivity(idleActivity());
}

void PresencePublisher::onStateChanged(const QString &state) {
    // "encounter"/"dungeon" transitions are handled by the more specific
    // signals above (ManagerService emits stateChanged first, then the
    // detail signal, so those win regardless of connection order).
    if (state == QStringLiteral("idle")) {
        m_client.clearActivity();
    } else if (state == QStringLiteral("watching")) {
        m_client.setActivity(idleActivity());
    }
}
