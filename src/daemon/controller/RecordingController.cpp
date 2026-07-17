#include "RecordingController.h"

using namespace Qt::StringLiterals;

RecordingController::RecordingController(Config config, QObject* parent)
    : QObject(parent), m_config(std::move(config))
{
    m_overrunTimer.setSingleShot(true);
    connect(&m_overrunTimer, &QTimer::timeout, this, &RecordingController::onOverrunElapsed);
    m_dungeonOverrunTimer.setSingleShot(true);
    connect(
        &m_dungeonOverrunTimer, &QTimer::timeout, this,
        &RecordingController::onDungeonOverrunElapsed
    );
}

std::optional<RecordingController::RaidDifficulty> RecordingController::raidDifficultyFromId(
    int difficultyId
)
{
    switch (difficultyId)
    {
        case 17:
            return RaidDifficulty::LFR;
        case 14:
            return RaidDifficulty::Normal;
        case 15:
            return RaidDifficulty::Heroic;
        case 16:
            return RaidDifficulty::Mythic;
        default:
            return std::nullopt;
    }
}

void RecordingController::onLineReceived(LogLine const& line)
{
    if (!line.isValid())
        return;

    QString const type = line.type();
    if (type == u"ENCOUNTER_START"_s)
    {
        // Inside an active M+ key, boss pulls are sub-segment chapters of
        // the dungeon recording, not separate recordings (PLAN.md §3.4).
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
        // MAP_CHANGE args: uiMapID, zoneName, x, y, z, w (confirmed against
        // real logs; not in the original RFC-001 grammar).
        if (line.argCount() >= 3)
            Q_EMIT zoneChanged(line.argString(1).toInt(), line.argString(2));
    }
}

void RecordingController::handleEncounterStart(LogLine const& line)
{
    // ENCOUNTER_START args: encounterID, encounterName, difficultyID,
    // groupSize, instanceID (PLAN.md §6).
    if (line.argCount() < 5)
        return;

    if (m_active)
    {
        // A new pull always follows an ENCOUNTER_END for the previous one,
        // so if we're still "active" it means the previous encounter's
        // overrun tail hasn't finished yet (a quick re-pull). Cut it short
        // now rather than making the new pull wait — the underlying replay
        // buffer never stops, this only affects where we call the previous
        // segment finished.
        if (m_overrunTimer.isActive())
        {
            m_overrunTimer.stop();
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
    Q_EMIT recordingStarted(m_current, preRollFrom);
}

void RecordingController::handleEncounterEnd(LogLine const& line)
{
    // ENCOUNTER_END args: encounterID, encounterName, difficultyID,
    // groupSize, success(0/1) (PLAN.md §6).
    if (!m_active || line.argCount() < 5)
        return;

    int const encounterId = line.argString(1).toInt();
    if (encounterId != m_current.encounterId)
    {
        // Stray END that doesn't match the encounter we're tracking; ignore
        // rather than risk stopping the wrong recording.
        return;
    }

    m_pendingSuccess = line.argString(5) == u"1"_s;
    m_pendingStopTime = line.dateTime().addSecs(m_config.raidOverrunSeconds);
    m_overrunTimer.start(m_config.raidOverrunSeconds * 1000);
}

void RecordingController::onOverrunElapsed()
{
    finishPendingStop();
    m_active = false;
}

void RecordingController::finishPendingStop()
{
    Q_EMIT recordingStopped(m_current, m_pendingSuccess, m_pendingStopTime);
}

void RecordingController::handleChallengeModeStart(LogLine const& line)
{
    // CHALLENGE_MODE_START args: zoneName, zoneID, mapID, keystoneLevel,
    // affixes[] (PLAN.md §6; confirmed against real logs).
    if (line.argCount() < 5)
        return;

    if (m_dungeonActive)
    {
        if (m_dungeonOverrunTimer.isActive())
        {
            // A CHALLENGE_MODE_END always precedes a genuinely new key,
            // so a start while the previous key's overrun tail is still
            // pending means that key ended and a new one began quickly.
            // Cut the tail short rather than making the new key wait.
            m_dungeonOverrunTimer.stop();
            finishPendingDungeonStop();
            m_dungeonActive = false;
        }
        else
        {
            // No pending END: this is zoning in/out of the same still-active
            // key re-firing the start event. Ignore.
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

void RecordingController::handleChallengeModeEnd(LogLine const& line)
{
    // CHALLENGE_MODE_END args: mapID, success(0/1), keystoneLevel,
    // durationMs, plus trailing fields PLAN.md §6 doesn't mention (confirmed
    // against real logs; unused here — see HANDOFF.md).
    if (!m_dungeonActive || line.argCount() < 5)
        return;

    m_pendingDungeonSuccess = line.argString(2) == u"1"_s;
    m_pendingDungeonDurationMs = line.argString(4).toInt();
    m_pendingDungeonStopTime = line.dateTime().addSecs(m_config.dungeonOverrunSeconds);
    m_dungeonOverrunTimer.start(m_config.dungeonOverrunSeconds * 1000);
}

void RecordingController::onDungeonOverrunElapsed()
{
    finishPendingDungeonStop();
    m_dungeonActive = false;
}

void RecordingController::finishPendingDungeonStop()
{
    Q_EMIT dungeonStopped(
        m_currentDungeon, m_pendingDungeonSuccess, m_pendingDungeonDurationMs,
        m_pendingDungeonStopTime
    );
}
