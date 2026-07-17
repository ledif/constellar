#pragma once

#include <QDateTime>
#include <QVariantMap>

#include "ActivityTracker.h"

// Translates ActivityTracker's decision types (RaidEncounter/DungeonRun) into
// the QVariantMap metadata dictionaries GameState stores, using the key
// vocabulary in ActivityKeys.h (ADR-012). Split out of ObserverService so the
// translation (e.g. raidDifficultyDisplayName's WoW-difficultyID ->
// display-name mapping) is unit-testable without constructing the whole
// LogWatcher/ActivityTracker/GameState pipeline.
class ActivityMetadataBuilder
{
  public:
    static QVariantMap encounterMetadata(ActivityTracker::RaidEncounter const& encounter);
    static QVariantMap encounterEndedMetadata(
        ActivityTracker::RaidEncounter const& encounter, bool success, QDateTime const& stopTime
    );

    static QVariantMap dungeonMetadata(ActivityTracker::DungeonRun const& dungeon);
    static QVariantMap dungeonEndedMetadata(
        ActivityTracker::DungeonRun const& dungeon, bool success, int durationMs,
        QDateTime const& stopTime
    );

    static QVariantMap zoneMetadata(int mapId, QString const& zoneName);

  private:
    static QString raidDifficultyDisplayName(int difficultyId);
};
