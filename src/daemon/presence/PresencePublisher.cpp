#include "PresencePublisher.h"

#include <QTimeZone>

#include "ActivityKeys.h"
#include "DiscordIpcClient.h"
#include "GameState.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

namespace
{

QString const kLargeImageKey = u"homestone"_s;

void addDefaultAssets(QJsonObject& activity)
{
    QJsonObject assets;
    assets["large_image"] = kLargeImageKey;
    activity["assets"] = assets;
}

qint64 startTimeSecs(QVariantMap const& activity)
{
    qint64 const startMs = activity.value(QString::fromLatin1(keys::kStartTime)).toLongLong();
    return QDateTime::fromMSecsSinceEpoch(startMs, QTimeZone::UTC).toSecsSinceEpoch();
}

}  // namespace

PresencePublisher::PresencePublisher(
    DiscordIpcClient& client, GameState const& gameState, QObject* parent
)
    : QObject(parent), m_client(client), m_gameState(gameState)
{
}

QJsonObject PresencePublisher::encounterActivity(
    QVariantMap const& activity, QString const& zoneName
)
{
    QString const encounterName =
        activity.value(QString::fromLatin1(keys::kEncounterName)).toString();
    QString const difficulty = activity.value(QString::fromLatin1(keys::kDifficulty)).toString();

    QJsonObject result;
    result["details"] = u"%1 %2"_s.arg(difficulty, encounterName);
    result["state"] = zoneName.isEmpty() ? u"Raid Encounter"_s : zoneName;
    QJsonObject timestamps;
    timestamps["start"] = startTimeSecs(activity);
    result["timestamps"] = timestamps;
    addDefaultAssets(result);
    return result;
}

QJsonObject PresencePublisher::dungeonActivity(QVariantMap const& activity)
{
    uint const keystoneLevel = activity.value(QString::fromLatin1(keys::kKeystoneLevel)).toUInt();

    QJsonObject result;
    result["details"] = u"Mythic+ Key +%1"_s.arg(keystoneLevel);
    result["state"] = u"In a dungeon"_s;
    QJsonObject timestamps;
    timestamps["start"] = startTimeSecs(activity);
    result["timestamps"] = timestamps;
    addDefaultAssets(result);
    return result;
}

QJsonObject PresencePublisher::idleActivity(QString const& zoneName)
{
    QJsonObject activity;
    activity["details"] = u"In World of Warcraft"_s;

    if (!zoneName.isEmpty())
        activity["state"] = zoneName;

    addDefaultAssets(activity);
    return activity;
}

QJsonObject PresencePublisher::activityFor(QVariantMap const& activity, QVariantMap const& zone)
{
    QString const zoneName = zone.value(QString::fromLatin1(keys::kZoneName)).toString();
    if (activity.isEmpty())
        return idleActivity(zoneName);

    QString const type = activity.value(QString::fromLatin1(keys::kType)).toString();

    if (type == QString::fromLatin1(keys::kTypeEncounter))
        return encounterActivity(activity, zoneName);

    if (type == QString::fromLatin1(keys::kTypeDungeon))
        return dungeonActivity(activity);

    return idleActivity(zoneName);
}

void PresencePublisher::onActivityChanged(QVariantMap const& /*activity*/)
{
    updatePresence();
}

void PresencePublisher::onZoneChanged(QVariantMap const& /*zone*/)
{
    updatePresence();
}

void PresencePublisher::updatePresence()
{
    m_client.setActivity(activityFor(m_gameState.activity(), m_gameState.zone()));
}
