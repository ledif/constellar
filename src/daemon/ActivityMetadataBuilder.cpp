#include "ActivityMetadataBuilder.h"

#include "ActivityKeys.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

QString ActivityMetadataBuilder::raidDifficultyDisplayName(int difficultyId)
{
    std::optional<ActivityTracker::RaidDifficulty> const difficulty =
        ActivityTracker::raidDifficultyFromId(difficultyId);
    if (!difficulty)
        return u"Unknown"_s;
    switch (*difficulty)
    {
        case ActivityTracker::RaidDifficulty::LFR:
            return u"LFR"_s;
        case ActivityTracker::RaidDifficulty::Normal:
            return u"Normal"_s;
        case ActivityTracker::RaidDifficulty::Heroic:
            return u"Heroic"_s;
        case ActivityTracker::RaidDifficulty::Mythic:
            return u"Mythic"_s;
    }
    return u"Unknown"_s;
}

QVariantMap ActivityMetadataBuilder::encounterMetadata(
    ActivityTracker::RaidEncounter const& encounter
)
{
    QVariantMap metadata;
    metadata[keys::kType] = QString::fromLatin1(keys::kTypeEncounter);
    metadata[keys::kEncounterId] = static_cast<uint>(encounter.encounterId);
    metadata[keys::kEncounterName] = encounter.encounterName;
    metadata[keys::kDifficulty] = raidDifficultyDisplayName(encounter.difficultyId);
    metadata[keys::kDifficultyId] = static_cast<uint>(encounter.difficultyId);
    metadata[keys::kStartTime] = static_cast<qint64>(encounter.startTime.toMSecsSinceEpoch());
    return metadata;
}

QVariantMap ActivityMetadataBuilder::encounterEndedMetadata(
    ActivityTracker::RaidEncounter const& encounter, bool success, QDateTime const& stopTime
)
{
    QVariantMap metadata = encounterMetadata(encounter);
    metadata[keys::kSuccess] = success;
    metadata[keys::kStopTime] = static_cast<qint64>(stopTime.toMSecsSinceEpoch());
    return metadata;
}

QVariantMap ActivityMetadataBuilder::dungeonMetadata(ActivityTracker::DungeonRun const& dungeon)
{
    QVariantMap metadata;
    metadata[keys::kType] = QString::fromLatin1(keys::kTypeDungeon);
    metadata[keys::kMapId] = static_cast<uint>(dungeon.mapId);
    metadata[keys::kZoneId] = static_cast<uint>(dungeon.zoneId);
    metadata[keys::kKeystoneLevel] = static_cast<uint>(dungeon.keystoneLevel);
    metadata[keys::kStartTime] = static_cast<qint64>(dungeon.startTime.toMSecsSinceEpoch());
    return metadata;
}

QVariantMap ActivityMetadataBuilder::dungeonEndedMetadata(
    ActivityTracker::DungeonRun const& dungeon, bool success, int durationMs,
    QDateTime const& stopTime
)
{
    QVariantMap metadata = dungeonMetadata(dungeon);
    metadata[keys::kSuccess] = success;
    metadata[keys::kDurationMs] = static_cast<qint64>(durationMs);
    metadata[keys::kStopTime] = static_cast<qint64>(stopTime.toMSecsSinceEpoch());
    return metadata;
}

QVariantMap ActivityMetadataBuilder::zoneMetadata(int mapId, QString const& zoneName)
{
    return QVariantMap{{keys::kMapId, static_cast<uint>(mapId)}, {keys::kZoneName, zoneName}};
}
