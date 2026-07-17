#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariantMap>

class DiscordIpcClient;
class GameState;

// Consumes GameState's Activity/Zone metadata (ADR-012) and turns them
// into Discord Rich Presence payloads pushed through a DiscordIpcClient
// (RFC-002). A sibling of ActivityTracker, not a dependent -- it only
// reads GameState, which ObserverService already fills in. A pure
// projection: no shadow state of its own.
//
// v1 scope: details/state/timestamps only. large_image/small_image/buttons
// need Developer Portal asset keys that don't exist yet (RFC-002 appendix);
// they're a later pass, not a functional gap in this MVP.
class PresencePublisher : public QObject
{
    Q_OBJECT

  public:
    explicit PresencePublisher(
        DiscordIpcClient& client, GameState const& gameState, QObject* parent = nullptr
    );

    // Pure mapping helpers, exposed for testing without a live socket.
    // encounterActivity/dungeonActivity take the Activity metadata
    // (ActivityKeys.h keys); zoneName is the Zone metadata's zoneName, empty
    // if no MAP_CHANGE has been seen yet -- falls back to a generic state.
    static QJsonObject encounterActivity(QVariantMap const& activity, QString const& zoneName = {});
    static QJsonObject dungeonActivity(QVariantMap const& activity);
    static QJsonObject idleActivity(QString const& zoneName = {});

    // The full activity+zone -> Discord payload mapping, as one pure
    // function. Recomputing from both on every change (rather than
    // patching in place) is what makes a Zone-only change unable to clobber
    // an in-progress Activity -- it only falls back to idle when activity
    // is empty.
    static QJsonObject activityFor(QVariantMap const& activity, QVariantMap const& zone);

  public Q_SLOTS:
    void onActivityChanged(QVariantMap const& activity);
    void onZoneChanged(QVariantMap const& zone);

  private:
    void updatePresence();

    DiscordIpcClient& m_client;
    GameState const& m_gameState;
};
