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

QVariantMap zoneBag(QString const& zoneName)
{
    return QVariantMap{{keys::kZoneName, zoneName}};
}

}  // namespace

void PresencePublisherTest::encounterActivityMapsDifficultyAndName()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    QJsonObject const activity = PresencePublisher::encounterActivity(
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start)
    );

    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic Ulgrax the Devourer"));
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
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start),
        QStringLiteral("Nerub-ar Palace")
    );

    QCOMPARE(activity.value("state").toString(), QStringLiteral("Nerub-ar Palace"));
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
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start)
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

// Regression test for the live bug ADR-012/TASK-001 fixed by construction:
// onZoneChanged used to call m_client.setActivity(idleActivity(...))
// unconditionally, so a MAP_CHANGE mid-pull clobbered encounter presence
// with idle. activityFor() recomputes from both every time and only
// falls back to idle when Activity is empty, so this can't happen anymore.
void PresencePublisherTest::activityForKeepsEncounterAcrossZoneChange()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    QVariantMap const activity =
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start);
    QVariantMap const zone = zoneBag(QStringLiteral("Nerub-ar Palace"));

    QJsonObject const result = PresencePublisher::activityFor(activity, zone);

    QCOMPARE(result.value("details").toString(), QStringLiteral("Mythic Ulgrax the Devourer"));
    QCOMPARE(result.value("state").toString(), QStringLiteral("Nerub-ar Palace"));
}

void PresencePublisherTest::activityForFallsBackToIdleWhenActivityEmpty()
{
    QVariantMap const zone = zoneBag(QStringLiteral("Dornogal"));

    QJsonObject const result = PresencePublisher::activityFor(QVariantMap{}, zone);

    QCOMPARE(result, PresencePublisher::idleActivity(QStringLiteral("Dornogal")));
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
    connect(&gameState, &GameState::zoneChanged, &publisher, &PresencePublisher::onZoneChanged);

    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    gameState.setActivity(
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start)
    );

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QCOMPARE(
        payload.value("args").toObject().value("activity").toObject().value("details").toString(),
        QStringLiteral("Mythic Ulgrax the Devourer")
    );

    // This update lands inside the throttle window, so it's coalesced away.
    gameState.setZone(zoneBag(QStringLiteral("Nerub-ar Palace")));

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QJsonObject const activity = payload.value("args").toObject().value("activity").toObject();
    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic Ulgrax the Devourer"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("Nerub-ar Palace"));
}

QTEST_MAIN(PresencePublisherTest)
