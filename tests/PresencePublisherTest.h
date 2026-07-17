#pragma once

#include <QObject>

// Covers PresencePublisher's pure state->activity mapping only. The
// signal-handling side (onActivityChanged/onZoneChanged) is a thin
// pass-through to activityFor()+DiscordIpcClient::setActivity, exercised
// end-to-end by DiscordIpcClientTest instead of re-mocked here.
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
};
