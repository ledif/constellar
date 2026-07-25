#include "ActivityProjectionTest.h"

#include <QDBusObjectPath>
#include <QDateTime>
#include <QTest>

#include "ActivityMetadata.h"
#include "ActivityProjection.h"

using namespace constellar::observer;

void ActivityProjectionTest::encounterProjectsActivityAndEncounterInterfaces()
{
    RaidEncounter const encounter{
        .encounterId = 3182,
        .encounterName = QStringLiteral("Belo'ren, Child of Al'ar"),
        .difficultyId = 14,  // Normal
        .startTime = QDateTime::fromMSecsSinceEpoch(1000),
    };
    ActivityMetadata const bag = ActivityMetadata::fromEncounter(encounter);

    QVERIFY(isEncounter(bag));
    QVERIFY(!isDungeon(bag));

    QVariantMap const activity = activityInterfaceProperties(bag);
    QCOMPARE(activity.value(QStringLiteral("Type")).toString(), QStringLiteral("encounter"));
    QCOMPARE(activity.value(QStringLiteral("StartTime")).toLongLong(), 1000);
    QCOMPARE(activity.value(QStringLiteral("Outcome")).toString(), QStringLiteral("unknown"));

    QVariantMap const encounterProps = encounterInterfaceProperties(bag);
    QCOMPARE(encounterProps.value(QStringLiteral("EncounterId")).toUInt(), 3182u);
    QCOMPARE(
        encounterProps.value(QStringLiteral("EncounterName")).toString(),
        QStringLiteral("Belo'ren, Child of Al'ar")
    );
    QCOMPARE(
        encounterProps.value(QStringLiteral("Difficulty")).toString(), QStringLiteral("Normal")
    );
    QCOMPARE(encounterProps.value(QStringLiteral("DifficultyId")).toUInt(), 14u);
}

void ActivityProjectionTest::dungeonProjectsActivityAndDungeonInterfaces()
{
    DungeonRun const dungeon{
        .zoneId = 2811,
        .challengeMapId = 558,
        .keystoneLevel = 10,
        .startTime = QDateTime::fromMSecsSinceEpoch(2000),
    };
    ActivityMetadata const bag = ActivityMetadata::fromDungeon(dungeon);

    QVERIFY(isDungeon(bag));
    QVERIFY(!isEncounter(bag));

    QVariantMap const activity = activityInterfaceProperties(bag);
    QCOMPARE(activity.value(QStringLiteral("Type")).toString(), QStringLiteral("dungeon"));

    QVariantMap const dungeonProps = dungeonInterfaceProperties(bag);
    QCOMPARE(dungeonProps.value(QStringLiteral("ZoneId")).toUInt(), 2811u);
    QCOMPARE(dungeonProps.value(QStringLiteral("KeystoneLevel")).toUInt(), 10u);
    QCOMPARE(dungeonProps.value(QStringLiteral("ChallengeMapId")).toUInt(), 558u);
}

void ActivityProjectionTest::unknownTypeDefaultsAreUnknown()
{
    QVariantMap const activity = activityInterfaceProperties(QVariantMap{});
    QCOMPARE(activity.value(QStringLiteral("Type")).toString(), QStringLiteral("unknown"));
    QCOMPARE(activity.value(QStringLiteral("Outcome")).toString(), QStringLiteral("unknown"));
}

void ActivityProjectionTest::recordingIsAlwaysTheNoObjectSentinel()
{
    RaidEncounter const encounter{.encounterId = 1, .encounterName = QStringLiteral("Test")};
    ActivityMetadata const bag = ActivityMetadata::fromEncounter(encounter);

    QVariantMap const activity = activityInterfaceProperties(bag);
    QCOMPARE(
        activity.value(QStringLiteral("Recording")).value<QDBusObjectPath>(),
        QDBusObjectPath(QStringLiteral("/"))
    );
}

QTEST_MAIN(ActivityProjectionTest)
