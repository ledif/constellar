#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariantMap>

class DiscordIpcClient;
class GameState;

class PresencePublisher : public QObject
{
    Q_OBJECT

  public:
    explicit PresencePublisher(
        DiscordIpcClient& client, GameState const& gameState, QObject* parent = nullptr
    );

    static QJsonObject encounterActivity(QVariantMap const& activity, QString const& zoneName = {});
    static QJsonObject dungeonActivity(QVariantMap const& activity);
    static QJsonObject idleActivity(QString const& zoneName = {});

    static QJsonObject activityFor(QVariantMap const& activity, QVariantMap const& zone);

  public Q_SLOTS:
    void onActivityChanged(QVariantMap const& activity);
    void onZoneChanged(QVariantMap const& zone);

  private:
    void updatePresence();

    DiscordIpcClient& m_client;
    GameState const& m_gameState;
};
