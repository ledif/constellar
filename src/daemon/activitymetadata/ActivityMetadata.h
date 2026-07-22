#pragma once

#include <QDateTime>
#include <QVariantMap>

#include "ActivityOutcome.h"
#include "DungeonRun.h"
#include "RaidEncounter.h"

class ActivityMetadata : public QVariantMap
{
  public:
    using QVariantMap::QVariantMap;

    static ActivityMetadata fromEncounter(RaidEncounter const& encounter);
    static ActivityMetadata fromEncounterEnded(
        RaidEncounter const& encounter, ActivityOutcome outcome, QDateTime const& stopTime
    );

    static ActivityMetadata fromDungeon(DungeonRun const& dungeon);
    static ActivityMetadata fromDungeonEnded(
        DungeonRun const& dungeon, ActivityOutcome outcome, int durationMs,
        QDateTime const& stopTime
    );
};
