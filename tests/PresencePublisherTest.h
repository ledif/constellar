#pragma once

#include <QObject>

class PresencePublisherTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void encounterActivityMapsDifficultyAndName();
    void encounterActivityUsesZoneNameAsState();
    void dungeonActivityMapsKeystoneLevel();
    void idleActivityHasNoTimestamp();
    void idleActivityOmitsStateWithoutZone();
    void activitiesIncludeLargeImageAsset();
    void activityForKeepsEncounterAcrossZoneChange();
    void activityForFallsBackToIdleWhenActivityEmpty();
    void activityForUsesZoneOnlyLocationAsState();
    void zoneChangeMidEncounterKeepsEncounterPresence();
};
