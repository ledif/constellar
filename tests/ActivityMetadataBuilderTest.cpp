#include "ActivityMetadataBuilderTest.h"

#include <QTest>

#include "ActivityKeys.h"
#include "ActivityMetadataBuilder.h"

namespace keys = constellar::keys;

void ActivityMetadataBuilderTest::encounterMetadataMapsDifficultyAndName()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    ActivityTracker::RaidEncounter const encounter{
        .encounterId = 2902,
        .encounterName = QStringLiteral("Ulgrax the Devourer"),
        .difficultyId = 16,  // Mythic
        .startTime = start,
    };

    QVariantMap const metadata = ActivityMetadataBuilder::encounterMetadata(encounter);

    QCOMPARE(metadata.value(keys::kType).toString(), QString::fromLatin1(keys::kTypeEncounter));
    QCOMPARE(metadata.value(keys::kEncounterId).toUInt(), 2902u);
    QCOMPARE(
        metadata.value(keys::kEncounterName).toString(), QStringLiteral("Ulgrax the Devourer")
    );
    QCOMPARE(metadata.value(keys::kDifficulty).toString(), QStringLiteral("Mythic"));
    QCOMPARE(metadata.value(keys::kDifficultyId).toUInt(), 16u);
    QCOMPARE(metadata.value(keys::kStartTime).toLongLong(), start.toMSecsSinceEpoch());
}

void ActivityMetadataBuilderTest::encounterMetadataUnknownDifficultyIdIsUnknown()
{
    ActivityTracker::RaidEncounter const encounter{
        .encounterId = 1,
        .encounterName = QStringLiteral("Test"),
        .difficultyId = 999,
    };

    QVariantMap const metadata = ActivityMetadataBuilder::encounterMetadata(encounter);

    QCOMPARE(metadata.value(keys::kDifficulty).toString(), QStringLiteral("Unknown"));
}

void ActivityMetadataBuilderTest::encounterEndedMetadataAddsSuccessAndStopTime()
{
    QDateTime const stopTime =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:45:00Z"), Qt::ISODate);
    ActivityTracker::RaidEncounter const encounter{
        .encounterId = 1,
        .encounterName = QStringLiteral("Test"),
        .difficultyId = 14,  // Normal
    };

    QVariantMap const metadata =
        ActivityMetadataBuilder::encounterEndedMetadata(encounter, true, stopTime);

    QCOMPARE(metadata.value(keys::kSuccess).toBool(), true);
    QCOMPARE(metadata.value(keys::kStopTime).toLongLong(), stopTime.toMSecsSinceEpoch());
    // Still carries the base encounter fields.
    QCOMPARE(metadata.value(keys::kDifficulty).toString(), QStringLiteral("Normal"));
}

void ActivityMetadataBuilderTest::dungeonMetadataMapsKeystoneLevel()
{
    QDateTime const start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);
    ActivityTracker::DungeonRun const dungeon{
        .zoneId = 501,
        .mapId = 2255,
        .keystoneLevel = 18,
        .startTime = start,
    };

    QVariantMap const metadata = ActivityMetadataBuilder::dungeonMetadata(dungeon);

    QCOMPARE(metadata.value(keys::kType).toString(), QString::fromLatin1(keys::kTypeDungeon));
    QCOMPARE(metadata.value(keys::kMapId).toUInt(), 2255u);
    QCOMPARE(metadata.value(keys::kZoneId).toUInt(), 501u);
    QCOMPARE(metadata.value(keys::kKeystoneLevel).toUInt(), 18u);
    QCOMPARE(metadata.value(keys::kStartTime).toLongLong(), start.toMSecsSinceEpoch());
}

void ActivityMetadataBuilderTest::dungeonEndedMetadataAddsSuccessDurationAndStopTime()
{
    QDateTime const stopTime =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:45:00Z"), Qt::ISODate);
    ActivityTracker::DungeonRun const dungeon{
        .zoneId = 501,
        .mapId = 2255,
        .keystoneLevel = 18,
    };

    QVariantMap const metadata =
        ActivityMetadataBuilder::dungeonEndedMetadata(dungeon, false, 1'234'567, stopTime);

    QCOMPARE(metadata.value(keys::kSuccess).toBool(), false);
    QCOMPARE(metadata.value(keys::kDurationMs).toLongLong(), 1'234'567);
    QCOMPARE(metadata.value(keys::kStopTime).toLongLong(), stopTime.toMSecsSinceEpoch());
}

void ActivityMetadataBuilderTest::zoneMetadataMapsMapIdAndZoneName()
{
    QVariantMap const metadata =
        ActivityMetadataBuilder::zoneMetadata(2214, QStringLiteral("March on Quel'Danas"));

    QCOMPARE(metadata.value(keys::kMapId).toUInt(), 2214u);
    QCOMPARE(metadata.value(keys::kZoneName).toString(), QStringLiteral("March on Quel'Danas"));
}

QTEST_MAIN(ActivityMetadataBuilderTest)
