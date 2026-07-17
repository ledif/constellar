#include "PresencePublisherTest.h"

#include <QJsonObject>
#include <QTest>

#include "ActivityKeys.h"
#include "PresencePublisher.h"

namespace keys = constellar::keys;

namespace {

QVariantMap encounterBag(const QString &encounterName, const QString &difficulty,
                         const QDateTime &start) {
    return QVariantMap{
        {keys::kType, QString::fromLatin1(keys::kTypeEncounter)},
        {keys::kEncounterName, encounterName},
        {keys::kDifficulty, difficulty},
        {keys::kStartTime, static_cast<qint64>(start.toMSecsSinceEpoch())},
    };
}

QVariantMap dungeonBag(uint keystoneLevel, const QDateTime &start) {
    return QVariantMap{
        {keys::kType, QString::fromLatin1(keys::kTypeDungeon)},
        {keys::kKeystoneLevel, keystoneLevel},
        {keys::kStartTime, static_cast<qint64>(start.toMSecsSinceEpoch())},
    };
}

QVariantMap zoneBag(const QString &zoneName) {
    return QVariantMap{{keys::kZoneName, zoneName}};
}

}  // namespace

void PresencePublisherTest::encounterActivityMapsDifficultyAndName() {
    const QDateTime start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    const QJsonObject activity = PresencePublisher::encounterActivity(
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start));

    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic Ulgrax the Devourer"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("Raid Encounter"));
    QCOMPARE(activity.value("timestamps").toObject().value("start").toInteger(),
             start.toUTC().toSecsSinceEpoch());
}

void PresencePublisherTest::encounterActivityUsesZoneNameAsState() {
    const QDateTime start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    const QJsonObject activity = PresencePublisher::encounterActivity(
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start),
        QStringLiteral("Nerub-ar Palace"));

    QCOMPARE(activity.value("state").toString(), QStringLiteral("Nerub-ar Palace"));
}

void PresencePublisherTest::dungeonActivityMapsKeystoneLevel() {
    const QDateTime start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    const QJsonObject activity = PresencePublisher::dungeonActivity(dungeonBag(18, start));

    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic+ Key +18"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("In a dungeon"));
    QCOMPARE(activity.value("timestamps").toObject().value("start").toInteger(),
             start.toUTC().toSecsSinceEpoch());
}

void PresencePublisherTest::idleActivityHasNoTimestamp() {
    const QJsonObject activity =
        PresencePublisher::idleActivity(QStringLiteral("March on Quel'Danas"));

    QCOMPARE(activity.value("details").toString(), QStringLiteral("In World of Warcraft"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("March on Quel'Danas"));
    QVERIFY(!activity.contains("timestamps"));
}

void PresencePublisherTest::idleActivityOmitsStateWithoutZone() {
    const QJsonObject activity = PresencePublisher::idleActivity();

    QVERIFY(!activity.contains("state"));
}

void PresencePublisherTest::activitiesIncludeLargeImageAsset() {
    const QDateTime start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    const QJsonObject encounter = PresencePublisher::encounterActivity(
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start));
    const QJsonObject dungeon = PresencePublisher::dungeonActivity(dungeonBag(18, start));
    const QJsonObject idle = PresencePublisher::idleActivity();

    for (const QJsonObject &activity : {encounter, dungeon, idle}) {
        QCOMPARE(activity.value("assets").toObject().value("large_image").toString(),
                 QStringLiteral("homestone"));
    }
}

// Regression test for the live bug ADR-012/TASK-001 fixed by construction:
// onZoneChanged used to call m_client.setActivity(idleActivity(...))
// unconditionally, so a MAP_CHANGE mid-pull clobbered encounter presence
// with idle. activityFor() recomputes from both bags every time and only
// falls back to idle when Activity is empty, so this can't happen anymore.
void PresencePublisherTest::activityForKeepsEncounterAcrossZoneChange() {
    const QDateTime start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    const QVariantMap activity =
        encounterBag(QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start);
    const QVariantMap zone = zoneBag(QStringLiteral("Nerub-ar Palace"));

    const QJsonObject result = PresencePublisher::activityFor(activity, zone);

    QCOMPARE(result.value("details").toString(), QStringLiteral("Mythic Ulgrax the Devourer"));
    QCOMPARE(result.value("state").toString(), QStringLiteral("Nerub-ar Palace"));
    QVERIFY(result.value("details").toString() != QStringLiteral("In World of Warcraft"));
}

void PresencePublisherTest::activityForFallsBackToIdleWhenActivityEmpty() {
    const QVariantMap zone = zoneBag(QStringLiteral("Dornogal"));

    const QJsonObject result = PresencePublisher::activityFor(QVariantMap{}, zone);

    QCOMPARE(result, PresencePublisher::idleActivity(QStringLiteral("Dornogal")));
}

QTEST_MAIN(PresencePublisherTest)
