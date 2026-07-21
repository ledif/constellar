#include "ActivityMetadata.h"

#include "ActivityKeys.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

namespace
{

QString raidDifficultyDisplayName(int difficultyId)
{
    std::optional<RaidDifficulty> const difficulty = raidDifficultyFromId(difficultyId);

    if (!difficulty)
        return u"Unknown"_s;

    switch (*difficulty)
    {
        case RaidDifficulty::LFR:
            return u"LFR"_s;
        case RaidDifficulty::Normal:
            return u"Normal"_s;
        case RaidDifficulty::Heroic:
            return u"Heroic"_s;
        case RaidDifficulty::Mythic:
            return u"Mythic"_s;
    }
    return u"Unknown"_s;
}

}  // namespace

ActivityMetadata ActivityMetadata::fromEncounter(RaidEncounter const& encounter)
{
    ActivityMetadata metadata;
    metadata[keys::kType] = QString::fromLatin1(keys::kTypeEncounter);
    metadata[keys::kEncounterId] = static_cast<uint>(encounter.encounterId);
    metadata[keys::kEncounterName] = encounter.encounterName;
    metadata[keys::kDifficulty] = raidDifficultyDisplayName(encounter.difficultyId);
    metadata[keys::kDifficultyId] = static_cast<uint>(encounter.difficultyId);
    metadata[keys::kStartTime] = static_cast<qint64>(encounter.startTime.toMSecsSinceEpoch());
    return metadata;
}

ActivityMetadata ActivityMetadata::fromEncounterEnded(
    RaidEncounter const& encounter, bool success, QDateTime const& stopTime
)
{
    ActivityMetadata metadata = fromEncounter(encounter);
    metadata[keys::kSuccess] = success;
    metadata[keys::kStopTime] = static_cast<qint64>(stopTime.toMSecsSinceEpoch());
    return metadata;
}

ActivityMetadata ActivityMetadata::fromDungeon(DungeonRun const& dungeon)
{
    ActivityMetadata metadata;
    metadata[keys::kType] = QString::fromLatin1(keys::kTypeDungeon);
    metadata[keys::kChallengeMapId] = static_cast<uint>(dungeon.challengeMapId);
    metadata[keys::kZoneId] = static_cast<uint>(dungeon.zoneId);
    metadata[keys::kKeystoneLevel] = static_cast<uint>(dungeon.keystoneLevel);
    metadata[keys::kStartTime] = static_cast<qint64>(dungeon.startTime.toMSecsSinceEpoch());
    return metadata;
}

ActivityMetadata ActivityMetadata::fromDungeonEnded(
    DungeonRun const& dungeon, bool success, int durationMs, QDateTime const& stopTime
)
{
    ActivityMetadata metadata = fromDungeon(dungeon);
    metadata[keys::kSuccess] = success;
    metadata[keys::kDurationMs] = static_cast<qint64>(durationMs);
    metadata[keys::kStopTime] = static_cast<qint64>(stopTime.toMSecsSinceEpoch());
    return metadata;
}
