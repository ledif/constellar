#pragma once

#include <QDateTime>
#include <QVariantMap>

#include "DungeonRun.h"
#include "RaidEncounter.h"

class ActivityMetadata : public QVariantMap
{
  public:
    using QVariantMap::QVariantMap;

    static ActivityMetadata fromEncounter(RaidEncounter const& encounter);
    static ActivityMetadata fromEncounterEnded(
        RaidEncounter const& encounter, bool success, QDateTime const& stopTime
    );

    static ActivityMetadata fromDungeon(DungeonRun const& dungeon);
    static ActivityMetadata fromDungeonEnded(
        DungeonRun const& dungeon, bool success, int durationMs, QDateTime const& stopTime
    );

    static ActivityMetadata fromZone(int mapId, QString const& zoneName);
};
