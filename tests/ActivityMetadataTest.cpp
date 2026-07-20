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
        .encounterId = 2902,
        .encounterName = QStringLiteral("Ulgrax the Devourer"),
        .difficultyId = 16,  // Mythic
        .startTime = start,
    };

    ActivityMetadata const metadata = ActivityMetadata::fromEncounter(encounter);

    QCOMPARE(metadata.value(keys::kType).toString(), QString::fromLatin1(keys::kTypeEncounter));
    QCOMPARE(metadata.value(keys::kEncounterId).toUInt(), 2902u);
    QCOMPARE(
        metadata.value(keys::kEncounterName).toString(), QStringLiteral("Ulgrax the Devourer")
    );
    QCOMPARE(metadata.value(keys::kDifficulty).toString(), QStringLiteral("Mythic"));
    QCOMPARE(metadata.value(keys::kDifficultyId).toUInt(), 16u);
    QCOMPARE(metadata.value(keys::kStartTime).toLongLong(), start.toMSecsSinceEpoch());
}

void ActivityMetadataTest::encounterMetadataUnknownDifficultyIdIsUnknown()
{
    RaidEncounter const encounter{
        .encounterId = 1,
        .encounterName = QStringLiteral("Test"),
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
        .encounterId = 1,
        .encounterName = QStringLiteral("Test"),
        .difficultyId = 14,  // Normal
    };

    ActivityMetadata const metadata =
        ActivityMetadata::fromEncounterEnded(encounter, true, stopTime);

    QCOMPARE(metadata.value(keys::kSuccess).toBool(), true);
    QCOMPARE(metadata.value(keys::kStopTime).toLongLong(), stopTime.toMSecsSinceEpoch());
    // Still carries the base encounter fields.
    QCOMPARE(metadata.value(keys::kDifficulty).toString(), QStringLiteral("Normal"));
}

void ActivityMetadataTest::dungeonMetadataMapsKeystoneLevel()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    DungeonRun const dungeon{
        .zoneId = 501,
        .mapId = 2255,
        .keystoneLevel = 18,
        .startTime = start,
    };

    ActivityMetadata const metadata = ActivityMetadata::fromDungeon(dungeon);

    QCOMPARE(metadata.value(keys::kType).toString(), QString::fromLatin1(keys::kTypeDungeon));
    QCOMPARE(metadata.value(keys::kMapId).toUInt(), 2255u);
    QCOMPARE(metadata.value(keys::kZoneId).toUInt(), 501u);
    QCOMPARE(metadata.value(keys::kKeystoneLevel).toUInt(), 18u);
    QCOMPARE(metadata.value(keys::kStartTime).toLongLong(), start.toMSecsSinceEpoch());
}

void ActivityMetadataTest::dungeonEndedMetadataAddsSuccessDurationAndStopTime()
{
    QDateTime const stopTime =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:45:00Z"), Qt::ISODate);
    DungeonRun const dungeon{
        .zoneId = 501,
        .mapId = 2255,
        .keystoneLevel = 18,
    };

    ActivityMetadata const metadata =
        ActivityMetadata::fromDungeonEnded(dungeon, false, 1'234'567, stopTime);

    QCOMPARE(metadata.value(keys::kSuccess).toBool(), false);
    QCOMPARE(metadata.value(keys::kDurationMs).toLongLong(), 1'234'567);
    QCOMPARE(metadata.value(keys::kStopTime).toLongLong(), stopTime.toMSecsSinceEpoch());
}

void ActivityMetadataTest::zoneMetadataMapsMapIdAndZoneName()
{
    ActivityMetadata const metadata =
        ActivityMetadata::fromZone(2214, QStringLiteral("March on Quel'Danas"));

    QCOMPARE(metadata.value(keys::kMapId).toUInt(), 2214u);
    QCOMPARE(metadata.value(keys::kZoneName).toString(), QStringLiteral("March on Quel'Danas"));
}

QTEST_MAIN(ActivityMetadataTest)
