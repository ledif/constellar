#include "ObserverService.h"

#include "ActivityMetadata.h"

ObserverService::ObserverService(
    std::filesystem::path const& logDirectory, ActivityTracker::Config config, QObject* parent
)
    : QObject(parent),
      m_logDirectory(logDirectory),
      m_watcher(logDirectory),
      m_tracker(std::move(config))
{
    // send lines to the activity tracker
    connect(&m_watcher, &LogWatcher::lineReceived, &m_tracker, &ActivityTracker::onLineReceived);

    // update our state when the tracker detects we moved zones
    connect(
        &m_tracker, &ActivityTracker::zoneChanged, this, [this](int mapId, QString const& zoneName)
        { m_gameState.setZone(ActivityMetadata::fromZone(mapId, zoneName)); }
    );

    // update our state when the tracker detects we started/ended a key
    connect(
        &m_tracker, &ActivityTracker::dungeonStarted, this,
        [this](DungeonRun const& dungeon, QDateTime const& /*preRollFrom*/)
        { m_gameState.setActivity(ActivityMetadata::fromDungeon(dungeon)); }
    );

    connect(
        &m_tracker, &ActivityTracker::dungeonStopped, this,
        [this](DungeonRun const& dungeon, bool success, int durationMs, QDateTime const& stopTime)
        {
            m_gameState.endActivity(
                ActivityMetadata::fromDungeonEnded(dungeon, success, durationMs, stopTime)
            );
        }
    );

    // update our state when the tracker detects we started/ended a raid pull
    connect(
        &m_tracker, &ActivityTracker::encounterStarted, this,
        [this](
            RaidEncounter const& encounter, QDateTime const& /*preRollFrom*/
        ) { m_gameState.setActivity(ActivityMetadata::fromEncounter(encounter)); }
    );

    connect(
        &m_tracker, &ActivityTracker::encounterStopped, this,
        [this](RaidEncounter const& encounter, bool success, QDateTime const& stopTime)
        {
            m_gameState.endActivity(
                ActivityMetadata::fromEncounterEnded(encounter, success, stopTime)
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
