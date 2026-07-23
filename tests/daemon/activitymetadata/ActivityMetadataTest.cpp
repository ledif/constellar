#include "ActivityMetadataTest.h"

#include <QTest>

#include "ActivityKeys.h"
#include "ActivityMetadata.h"

namespace keys = constellar::keys;

void ActivityMetadataTest::encounterMetadataMapsDifficultyAndName()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    RaidEncounter const encounter{
        .encounterId = 3306,
        .encounterName = QStringLiteral("Chimaerus the Undreamt God"),
        .difficultyId = 16,  // Mythic
        .startTime = start,
    };

    ActivityMetadata const metadata = ActivityMetadata::fromEncounter(encounter);

    QCOMPARE(metadata.value(keys::kType).toString(), QString::fromLatin1(keys::kTypeEncounter));
    QCOMPARE(metadata.value(keys::kEncounterId).toUInt(), 3306u);
    QCOMPARE(
        metadata.value(keys::kEncounterName).toString(),
        QStringLiteral("Chimaerus the Undreamt God")
    );
    QCOMPARE(metadata.value(keys::kDifficulty).toString(), QStringLiteral("Mythic"));
    QCOMPARE(metadata.value(keys::kDifficultyId).toUInt(), 16u);
    QCOMPARE(metadata.value(keys::kStartTime).toLongLong(), start.toMSecsSinceEpoch());
}

void ActivityMetadataTest::encounterMetadataUnknownDifficultyIdIsUnknown()
{
    RaidEncounter const encounter{
        .encounterId = 3177,
        .encounterName = QStringLiteral("Vorasius"),
        .difficultyId = 999,
    };

    ActivityMetadata const metadata = ActivityMetadata::fromEncounter(encounter);

    QCOMPARE(metadata.value(keys::kDifficulty).toString(), QStringLiteral("Unknown"));
}

void ActivityMetadataTest::encounterEndedMetadataAddsSuccessAndStopTime()
{
    QDateTime const stopTime =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:45:00Z"), Qt::ISODate);
    RaidEncounter const encounter{
        .encounterId = 3177,
        .encounterName = QStringLiteral("Vorasius"),
        .difficultyId = 14,  // Normal
    };

    ActivityMetadata const metadata = ActivityMetadata::fromEncounterEnded(
        encounter, ActivityOutcome::Success, 381'329, stopTime
    );

    QCOMPARE(metadata.value(keys::kOutcome).toString(), QString::fromLatin1(keys::kOutcomeSuccess));
    QCOMPARE(metadata.value(keys::kDurationMs).toLongLong(), 381'329);
    QCOMPARE(metadata.value(keys::kStopTime).toLongLong(), stopTime.toMSecsSinceEpoch());
    // Still carries the base encounter fields.
    QCOMPARE(metadata.value(keys::kDifficulty).toString(), QStringLiteral("Normal"));
}

void ActivityMetadataTest::dungeonMetadataMapsKeystoneLevel()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    DungeonRun const dungeon{
        .zoneId = 2811,
        .challengeMapId = 558,
        .keystoneLevel = 10,
        .startTime = start,
    };

    ActivityMetadata const metadata = ActivityMetadata::fromDungeon(dungeon);

    QCOMPARE(metadata.value(keys::kType).toString(), QString::fromLatin1(keys::kTypeDungeon));
    QCOMPARE(metadata.value(keys::kChallengeMapId).toUInt(), 558u);
    QCOMPARE(metadata.value(keys::kZoneId).toUInt(), 2811u);
    QCOMPARE(metadata.value(keys::kKeystoneLevel).toUInt(), 10u);
    QCOMPARE(metadata.value(keys::kStartTime).toLongLong(), start.toMSecsSinceEpoch());
}

void ActivityMetadataTest::dungeonEndedMetadataAddsSuccessDurationAndStopTime()
{
    QDateTime const stopTime =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:45:00Z"), Qt::ISODate);
    DungeonRun const dungeon{
        .zoneId = 2811,
        .challengeMapId = 558,
        .keystoneLevel = 10,
    };

    ActivityMetadata const metadata = ActivityMetadata::fromDungeonEnded(
        dungeon, ActivityOutcome::Abandoned, 1'234'567, stopTime
    );

    QCOMPARE(
        metadata.value(keys::kOutcome).toString(), QString::fromLatin1(keys::kOutcomeAbandoned)
    );
    QCOMPARE(metadata.value(keys::kDurationMs).toLongLong(), 1'234'567);
    QCOMPARE(metadata.value(keys::kStopTime).toLongLong(), stopTime.toMSecsSinceEpoch());
}

QTEST_MAIN(ActivityMetadataTest)
