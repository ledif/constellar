#include "PresencePublisher.h"

#include "DiscordIpcClient.h"

PresencePublisher::PresencePublisher(DiscordIpcClient &client, QObject *parent)
    : QObject(parent), m_client(client) {}

QJsonObject PresencePublisher::encounterActivity(const QString &encounterName,
                                                 const QString &difficulty,
                                                 const QDateTime &startTime,
                                                 const QString &zoneName) {
    QJsonObject activity;
    activity["details"] = QStringLiteral("%1 %2").arg(difficulty, encounterName);
    activity["state"] = zoneName.isEmpty() ? QStringLiteral("Raid Encounter") : zoneName;
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

QJsonObject PresencePublisher::idleActivity(const QString &zoneName) {
    QJsonObject activity;
    // realm/character still require the addon-channel hybrid (ADR-011);
    // zoneName comes for free from MAP_CHANGE (RecordingController).
    activity["details"] = QStringLiteral("In World of Warcraft");
    if (!zoneName.isEmpty()) {
        activity["state"] = zoneName;
    }
    return activity;
}

void PresencePublisher::onEncounterDetected(int /*encounterId*/, const QString &encounterName,
                                            const QString &difficulty, const QString &startTime) {
    m_client.setActivity(encounterActivity(encounterName, difficulty,
                                           QDateTime::fromString(startTime, Qt::ISODateWithMs),
                                           m_currentZoneName));
}

void PresencePublisher::onEncounterEnded(int /*encounterId*/, const QString & /*encounterName*/,
                                         bool /*success*/, const QString & /*stopTime*/) {
    m_client.setActivity(idleActivity(m_currentZoneName));
}

void PresencePublisher::onDungeonDetected(int /*zoneId*/, int /*mapId*/, int keystoneLevel,
                                          const QString &startTime) {
    m_client.setActivity(
        dungeonActivity(keystoneLevel, QDateTime::fromString(startTime, Qt::ISODateWithMs)));
}

void PresencePublisher::onDungeonEnded(int /*mapId*/, int /*keystoneLevel*/, bool /*success*/,
                                       int /*durationMs*/, const QString & /*stopTime*/) {
    m_client.setActivity(idleActivity(m_currentZoneName));
}

void PresencePublisher::onStateChanged(const QString &state) {
    // "encounter"/"dungeon" transitions are handled by the more specific
    // signals above (ManagerService emits stateChanged first, then the
    // detail signal, so those win regardless of connection order).
    //
    // Deliberately NOT clearing on "idle": that state also fires from
    // LogWatcher's 60s no-write timeout, which trips during ordinary quiet
    // stretches mid-raid (running a corridor, waiting on a pull) -- treating
    // it as "player quit WoW" made presence flicker away constantly. Presence
    // now only changes on an actual zone change or encounter/dungeon
    // transition; it's left stale (rather than cleared) if WoW really does
    // exit, which is an acceptable v1 tradeoff (RFC-002 has no clean
    // "logged out" signal yet).
    if (state == QStringLiteral("watching")) {
        m_client.setActivity(idleActivity(m_currentZoneName));
    }
}

void PresencePublisher::onZoneChanged(int /*mapId*/, const QString &zoneName) {
    m_currentZoneName = zoneName;
    m_client.setActivity(idleActivity(m_currentZoneName));
}
