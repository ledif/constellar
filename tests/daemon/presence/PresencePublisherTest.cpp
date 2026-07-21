#include "PresencePublisherTest.h"

#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTemporaryDir>
#include <QTest>

#include "ActivityKeys.h"
#include "DiscordFrame.h"
#include "DiscordIpcClient.h"
#include "GameState.h"
#include "Location.h"
#include "PresencePublisher.h"

namespace keys = constellar::keys;

namespace
{

QVariantMap encounterBag(
    QString const& encounterName, QString const& difficulty, QDateTime const& start
)
{
    return QVariantMap{
        {keys::kType, QString::fromLatin1(keys::kTypeEncounter)},
        {keys::kEncounterName, encounterName},
        {keys::kDifficulty, difficulty},
        {keys::kStartTime, static_cast<qint64>(start.toMSecsSinceEpoch())},
    };
}

QVariantMap dungeonBag(uint keystoneLevel, QDateTime const& start)
{
    return QVariantMap{
        {keys::kType, QString::fromLatin1(keys::kTypeDungeon)},
        {keys::kKeystoneLevel, keystoneLevel},
        {keys::kStartTime, static_cast<qint64>(start.toMSecsSinceEpoch())},
    };
}

QVariantMap uiMapOnlyLocationBag(QString const& uiMapName)
{
    Location location;
    location.setUiMap(UiMap{0, uiMapName});
    return location.toVariantMap();
}

QVariantMap zoneOnlyLocationBag(QString const& zoneName)
{
    Location location;
    location.setZone(Zone{zoneName, 0});
    return location.toVariantMap();
}

}  // namespace

void PresencePublisherTest::encounterActivityMapsDifficultyAndName()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    QJsonObject const activity = PresencePublisher::encounterActivity(
        encounterBag(QStringLiteral("Midnight Falls"), QStringLiteral("Mythic"), start)
    );

    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic Midnight Falls"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("Raid Encounter"));
    QCOMPARE(
        activity.value("timestamps").toObject().value("start").toInteger(),
        start.toUTC().toSecsSinceEpoch()
    );
}

void PresencePublisherTest::encounterActivityUsesZoneNameAsState()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    QJsonObject const activity = PresencePublisher::encounterActivity(
        encounterBag(QStringLiteral("Midnight Falls"), QStringLiteral("Mythic"), start),
        QStringLiteral("March on Quel'Danas")
    );

    QCOMPARE(activity.value("state").toString(), QStringLiteral("March on Quel'Danas"));
}

void PresencePublisherTest::dungeonActivityMapsKeystoneLevel()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    QJsonObject const activity = PresencePublisher::dungeonActivity(dungeonBag(18, start));

    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic+ Key +18"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("In a dungeon"));
    QCOMPARE(
        activity.value("timestamps").toObject().value("start").toInteger(),
        start.toUTC().toSecsSinceEpoch()
    );
}

void PresencePublisherTest::idleActivityHasNoTimestamp()
{
    QJsonObject const activity =
        PresencePublisher::idleActivity(QStringLiteral("March on Quel'Danas"));

    QCOMPARE(activity.value("details").toString(), QStringLiteral("In World of Warcraft"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("March on Quel'Danas"));
    QVERIFY(!activity.contains("timestamps"));
}

void PresencePublisherTest::idleActivityOmitsStateWithoutZone()
{
    QJsonObject const activity = PresencePublisher::idleActivity();

    QVERIFY(!activity.contains("state"));
}

void PresencePublisherTest::activitiesIncludeLargeImageAsset()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    QJsonObject const encounter = PresencePublisher::encounterActivity(
        encounterBag(QStringLiteral("Midnight Falls"), QStringLiteral("Mythic"), start)
    );
    QJsonObject const dungeon = PresencePublisher::dungeonActivity(dungeonBag(18, start));
    QJsonObject const idle = PresencePublisher::idleActivity();

    for (QJsonObject const& activity : {encounter, dungeon, idle})
    {
        QCOMPARE(
            activity.value("assets").toObject().value("large_image").toString(),
            QStringLiteral("homestone")
        );
    }
}

void PresencePublisherTest::activityForKeepsEncounterAcrossZoneChange()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    QVariantMap const activity =
        encounterBag(QStringLiteral("Midnight Falls"), QStringLiteral("Mythic"), start);
    QVariantMap const location = uiMapOnlyLocationBag(QStringLiteral("March on Quel'Danas"));

    QJsonObject const result = PresencePublisher::activityFor(activity, location);

    QCOMPARE(result.value("details").toString(), QStringLiteral("Mythic Midnight Falls"));
    QCOMPARE(result.value("state").toString(), QStringLiteral("March on Quel'Danas"));
}

void PresencePublisherTest::activityForFallsBackToIdleWhenActivityEmpty()
{
    QVariantMap const location = uiMapOnlyLocationBag(QStringLiteral("Dornogal"));

    QJsonObject const result = PresencePublisher::activityFor(QVariantMap{}, location);

    QCOMPARE(result, PresencePublisher::idleActivity(QStringLiteral("Dornogal")));
}

void PresencePublisherTest::activityForUsesZoneOnlyLocationAsState()
{
    // The interior case: ZONE_CHANGE with no accompanying MAP_CHANGE, so only the
    // zone half of Location is set.
    QVariantMap const location = zoneOnlyLocationBag(QStringLiteral("Sanctum of Light"));

    QJsonObject const result = PresencePublisher::activityFor(QVariantMap{}, location);

    QCOMPARE(result.value("state").toString(), QStringLiteral("Sanctum of Light"));
}

void PresencePublisherTest::zoneChangeMidEncounterKeepsEncounterPresence()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    qputenv("XDG_RUNTIME_DIR", dir.path().toUtf8());

    QString const socketPath = dir.path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient::Config config;
    config.throttleIntervalMs = 50;
    DiscordIpcClient client(QStringLiteral("1"), config);
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));  // handshake

    QJsonObject readyEvt;
    readyEvt["cmd"] = QStringLiteral("DISPATCH");
    readyEvt["evt"] = QStringLiteral("READY");
    peer->write(encodeFrame(1, readyEvt));
    peer->flush();
    QTRY_VERIFY(client.isReady());

    GameState gameState;
    PresencePublisher publisher(client, gameState);
    connect(
        &gameState, &GameState::activityChanged, &publisher, &PresencePublisher::onActivityChanged
    );
    connect(
        &gameState, &GameState::locationChanged, &publisher, &PresencePublisher::onLocationChanged
    );

    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    gameState.setActivity(
        encounterBag(QStringLiteral("Midnight Falls"), QStringLiteral("Mythic"), start)
    );

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QCOMPARE(
        payload.value("args").toObject().value("activity").toObject().value("details").toString(),
        QStringLiteral("Mythic Midnight Falls")
    );

    // This update lands inside the throttle window, so it's coalesced away.
    gameState.setUiMap(UiMap{0, QStringLiteral("March on Quel'Danas")});

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QJsonObject const activity = payload.value("args").toObject().value("activity").toObject();
    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic Midnight Falls"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("March on Quel'Danas"));
}

QTEST_MAIN(PresencePublisherTest)
