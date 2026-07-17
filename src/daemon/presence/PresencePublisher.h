#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QObject>
#include <QString>

class DiscordIpcClient;

// Consumes ManagerService's primitive-typed game-state signals (the same
// ones relayed over DBus) and turns them into Discord Rich Presence
// payloads pushed through a DiscordIpcClient (RFC-002). A sibling of
// RecordingController, not a dependent -- it only reads the decisions
// ManagerService already broadcasts.
//
// v1 scope: details/state/timestamps only. large_image/small_image/buttons
// need Developer Portal asset keys that don't exist yet (RFC-002 appendix);
// they're a later pass, not a functional gap in this MVP.
class PresencePublisher : public QObject {
    Q_OBJECT

  public:
    explicit PresencePublisher(DiscordIpcClient &client, QObject *parent = nullptr);

    // Pure mapping helpers, exposed for testing without a live socket.
    // zoneName is the MAP_CHANGE-derived label (RecordingController), empty
    // if no MAP_CHANGE has been seen yet -- falls back to a generic state.
    static QJsonObject encounterActivity(const QString &encounterName, const QString &difficulty,
                                         const QDateTime &startTime, const QString &zoneName = {});
    static QJsonObject dungeonActivity(int keystoneLevel, const QDateTime &startTime);
    static QJsonObject idleActivity(const QString &zoneName = {});

  public Q_SLOTS:
    void onEncounterDetected(int encounterId, const QString &encounterName,
                             const QString &difficulty, const QString &startTime);
    void onEncounterEnded(int encounterId, const QString &encounterName, bool success,
                          const QString &stopTime);
    void onDungeonDetected(int zoneId, int mapId, int keystoneLevel, const QString &startTime);
    void onDungeonEnded(int mapId, int keystoneLevel, bool success, int durationMs,
                        const QString &stopTime);
    void onStateChanged(const QString &state);
    void onZoneChanged(int mapId, const QString &zoneName);

  private:
    DiscordIpcClient &m_client;
    QString m_currentZoneName;
};
