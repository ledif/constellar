#pragma once

#include <QDateTime>
#include <QVariantMap>

#include "ActivityTracker.h"

class ActivityMetadata : public QVariantMap
{
  public:
    using QVariantMap::QVariantMap;

    static ActivityMetadata fromEncounter(ActivityTracker::RaidEncounter const& encounter);
    static ActivityMetadata fromEncounterEnded(
        ActivityTracker::RaidEncounter const& encounter, bool success, QDateTime const& stopTime
    );

    static ActivityMetadata fromDungeon(ActivityTracker::DungeonRun const& dungeon);
    static ActivityMetadata fromDungeonEnded(
        ActivityTracker::DungeonRun const& dungeon, bool success, int durationMs,
        QDateTime const& stopTime
    );

    static ActivityMetadata fromZone(int mapId, QString const& zoneName);
};
