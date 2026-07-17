#include "ObserverService.h"

#include "ActivityMetadataBuilder.h"

ObserverService::ObserverService(std::filesystem::path const& logDirectory, QObject* parent)
    : QObject(parent),
      m_logDirectory(logDirectory),
      m_watcher(logDirectory),
      m_tracker(ActivityTracker::Config{})
{
    // send lines to the activity tracker
    connect(&m_watcher, &LogWatcher::lineReceived, &m_tracker, &ActivityTracker::onLineReceived);

    // update our state when the tracker detects we moved zones
    connect(
        &m_tracker, &ActivityTracker::zoneChanged, this, [this](int mapId, QString const& zoneName)
        { m_gameState.setZone(ActivityMetadataBuilder::zoneMetadata(mapId, zoneName)); }
    );

    // update our state when the tracker detects we started/ended a key
    connect(
        &m_tracker, &ActivityTracker::dungeonStarted, this,
        [this](ActivityTracker::DungeonRun const& dungeon, QDateTime const& /*preRollFrom*/)
        { m_gameState.setActivity(ActivityMetadataBuilder::dungeonMetadata(dungeon)); }
    );

    connect(
        &m_tracker, &ActivityTracker::dungeonStopped, this,
        [this](
            ActivityTracker::DungeonRun const& dungeon, bool success, int durationMs,
            QDateTime const& stopTime
        )
        {
            m_gameState.endActivity(
                ActivityMetadataBuilder::dungeonEndedMetadata(
                    dungeon, success, durationMs, stopTime
                )
            );
        }
    );

    // update our state when the tracker detects we started/ended a raid pull
    connect(
        &m_tracker, &ActivityTracker::encounterStarted, this,
        [this](
            ActivityTracker::RaidEncounter const& encounter, QDateTime const& /*preRollFrom*/
        ) { m_gameState.setActivity(ActivityMetadataBuilder::encounterMetadata(encounter)); }
    );

    connect(
        &m_tracker, &ActivityTracker::encounterStopped, this,
        [this](
            ActivityTracker::RaidEncounter const& encounter, bool success, QDateTime const& stopTime
        )
        {
            m_gameState.endActivity(
                ActivityMetadataBuilder::encounterEndedMetadata(encounter, success, stopTime)
            );
        }
    );
}

bool ObserverService::start()
{
    return m_watcher.start();
}

GameState const& ObserverService::gameState() const
{
    return m_gameState;
}
