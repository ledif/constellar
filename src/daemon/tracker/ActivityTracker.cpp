#include "ActivityTracker.h"

using namespace Qt::StringLiterals;

ActivityTracker::ActivityTracker(Config config, QObject* parent)
    : QObject(parent), m_config(std::move(config))
{
    m_overrunTimer.setSingleShot(true);
    connect(&m_overrunTimer, &QTimer::timeout, this, &ActivityTracker::onOverrunElapsed);
    m_dungeonOverrunTimer.setSingleShot(true);
    connect(
        &m_dungeonOverrunTimer, &QTimer::timeout, this, &ActivityTracker::onDungeonOverrunElapsed
    );
}

void ActivityTracker::onLineReceived(LogLine const& line)
{
    if (!line.isValid())
        return;

    QString const type = line.type();
    if (type == u"ENCOUNTER_START"_s)
    {
        if (!m_dungeonActive)
            handleEncounterStart(line);
    }
    else if (type == u"ENCOUNTER_END"_s)
    {
        if (!m_dungeonActive)
            handleEncounterEnd(line);
    }
    else if (type == u"CHALLENGE_MODE_START"_s)
    {
        handleChallengeModeStart(line);
    }
    else if (type == u"CHALLENGE_MODE_END"_s)
    {
        handleChallengeModeEnd(line);
    }
    else if (type == u"MAP_CHANGE"_s)
    {
        // MAP_CHANGE args: uiMapID, uiMapName, x0, x1, y0, y1
        if (line.argCount() >= 3)
        {
            UiMap uiMap;
            uiMap.id = line.argString(1).toUInt();
            uiMap.name = line.argString(2);
            if (line.argCount() >= 7)
            {
                uiMap.bounds = MapBounds{
                    line.argString(3).toDouble(),
                    line.argString(4).toDouble(),
                    line.argString(5).toDouble(),
                    line.argString(6).toDouble(),
                };
            }
            Q_EMIT uiMapChanged(uiMap);
        }
    }
    else if (type == u"ZONE_CHANGE"_s)
    {
        // ZONE_CHANGE args: instanceID, zoneText, difficultyID
        if (line.argCount() >= 3)
        {
            Zone zone;
            zone.instanceId = line.argString(1).toUInt();
            zone.name = line.argString(2);
            zone.difficultyId = line.argString(3).toUInt();
            Q_EMIT zoneChanged(zone);
        }
    }
}

void ActivityTracker::handleEncounterStart(LogLine const& line)
{
    // ENCOUNTER_START args: encounterID, encounterName, difficultyID, groupSize, instanceID
    if (line.argCount() < 5)
        return;

    if (m_active)
    {
        if (m_overrunTimer.isActive())
        {
            // previous pull's overrun tail hadn't elapsed — flush its real END result
            m_overrunTimer.stop();
            finishPendingStop();
        }
        else
        {
            // fresh START with no END in between: the previous pull was a wipe
            m_pendingSuccess = false;
            m_pendingStopTime = line.dateTime();
            finishPendingStop();
        }
        m_active = false;
    }

    int const difficultyId = line.argString(3).toInt();
    std::optional<RaidDifficulty> const difficulty = raidDifficultyFromId(difficultyId);
    if (!difficulty || *difficulty < m_config.minDifficulty)
        return;

    m_current = RaidEncounter{
        line.argString(1).toInt(),
        line.argString(2),
        difficultyId,
        line.dateTime(),
    };

    m_active = true;

    QDateTime const preRollFrom = m_current.startTime.addSecs(-m_config.preRollSeconds);
    Q_EMIT encounterStarted(m_current, preRollFrom);
}

void ActivityTracker::handleEncounterEnd(LogLine const& line)
{
    // ENCOUNTER_END args: encounterID, encounterName, difficultyID,
    // groupSize, success(0/1)
    if (!m_active || line.argCount() < 5)
        return;

    int const encounterId = line.argString(1).toInt();
    if (encounterId != m_current.encounterId)
    {
        // stray END that doesn't match the encounter we're tracking
        // could this happen?
        return;
    }

    m_pendingSuccess = line.argString(5) == u"1"_s;
    m_pendingStopTime = line.dateTime().addSecs(m_config.raidOverrunSeconds);
    m_overrunTimer.start(m_config.raidOverrunSeconds * 1000);
}

void ActivityTracker::onOverrunElapsed()
{
    finishPendingStop();
    m_active = false;
}

void ActivityTracker::finishPendingStop()
{
    Q_EMIT encounterStopped(m_current, m_pendingSuccess, m_pendingStopTime);
}

void ActivityTracker::handleChallengeModeStart(LogLine const& line)
{
    // CHALLENGE_MODE_START args: zoneName, zoneID, challengeMapID, keystoneLevel, affixes[]
    if (line.argCount() < 5)
        return;

    if (m_dungeonActive)
    {
        if (m_dungeonOverrunTimer.isActive())
        {
            // how can this even happen?
            m_dungeonOverrunTimer.stop();
            finishPendingDungeonStop();
            m_dungeonActive = false;
        }
        else
        {
            return;
        }
    }

    int const level = line.argString(4).toInt();
    if (level < m_config.minKeystoneLevel)
        return;

    m_currentDungeon = DungeonRun{
        line.argString(2).toInt(),
        line.argString(3).toInt(),
        level,
        line.dateTime(),
    };

    m_dungeonActive = true;

    QDateTime const preRollFrom = m_currentDungeon.startTime.addSecs(-m_config.preRollSeconds);
    Q_EMIT dungeonStarted(m_currentDungeon, preRollFrom);
}

void ActivityTracker::handleChallengeModeEnd(LogLine const& line)
{
    // CHALLENGE_MODE_END args: zoneID, success(0/1), keystoneLevel, durationMs.
    // We don't match arg1 against the active key: you can only be in one key
    // at a time, so any END while a key is active ends it.
    if (!m_dungeonActive || line.argCount() < 5)
        return;

    m_pendingDungeonSuccess = line.argString(2) == u"1"_s;
    m_pendingDungeonDurationMs = line.argString(4).toInt();
    m_pendingDungeonStopTime = line.dateTime().addSecs(m_config.dungeonOverrunSeconds);
    m_dungeonOverrunTimer.start(m_config.dungeonOverrunSeconds * 1000);
}

void ActivityTracker::onDungeonOverrunElapsed()
{
    finishPendingDungeonStop();
    m_dungeonActive = false;
}

void ActivityTracker::finishPendingDungeonStop()
{
    Q_EMIT dungeonStopped(
        m_currentDungeon, m_pendingDungeonSuccess, m_pendingDungeonDurationMs,
        m_pendingDungeonStopTime
    );
}
