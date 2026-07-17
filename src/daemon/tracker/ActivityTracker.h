#pragma once

#include <optional>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

#include "LogLine.h"

// Raids + M+ first cut of the encounter state machine described in
// PLAN.md §3.4. Consumes parsed LogLines and decides when a raid pull or
// a Mythic+ key should be recorded — it does not touch libobs (that's
// ObsEngine, Phase 2) or SQLite (MetadataStore); it just emits start/stop
// decisions.
//
// MAP_CHANGE is tracked minimally (mapId + zone name only, via zoneChanged())
// since PresencePublisher needs a zone label for idle/raid presence
// (RFC-002); it's not a state-machine input like ENCOUNTER_*/CHALLENGE_MODE_*.
//
// Delves and ZONE_CHANGE handling are out of scope here. While a M+ key is
// active, nested ENCOUNTER_START/ENCOUNTER_END lines (boss sub-segments,
// per PLAN.md §3.4's "M+ nests encounters" rule) are consumed but produce
// no signal of their own yet — there's no timeline/chapter concept until
// MetadataStore exists.
class ActivityTracker : public QObject
{
    Q_OBJECT

  public:
    // Raid difficulties in increasing order of difficulty. WoW's numeric
    // difficultyIDs are NOT ordered this way (LFR=17 > Mythic=16), so we
    // can't threshold on the raw ID — this is a minimal stand-in for the
    // full instanceDifficulty table (PLAN.md §3.5/§6), covering just the
    // four raid difficulties needed to decide record-or-skip.
    enum class RaidDifficulty
    {
        LFR = 0,
        Normal = 1,
        Heroic = 2,
        Mythic = 3
    };

    struct Config
    {
        int preRollSeconds = 15;
        int raidOverrunSeconds = 20;
        RaidDifficulty minDifficulty = RaidDifficulty::Normal;
        int dungeonOverrunSeconds = 5;
        int minKeystoneLevel = 2;
    };

    struct RaidEncounter
    {
        int encounterId = 0;
        QString encounterName;
        int difficultyId = 0;
        QDateTime startTime;
    };

    struct DungeonRun
    {
        int zoneId = 0;
        int mapId = 0;
        int keystoneLevel = 0;
        QDateTime startTime;
    };

    explicit ActivityTracker(Config config, QObject* parent = nullptr);

    // Maps a WoW difficultyID to a RaidDifficulty rank, or nullopt if it's
    // not one of the four raid difficulties (e.g. a dungeon/M+ ID).
    static std::optional<RaidDifficulty> raidDifficultyFromId(int difficultyId);

  public Q_SLOTS:
    void onLineReceived(LogLine const& line);

  Q_SIGNALS:
    // Emitted the instant ENCOUNTER_START clears the difficulty threshold.
    // preRollFrom = startTime - Config::preRollSeconds. This isn't a
    // scheduling delay: the replay buffer runs continuously (PLAN.md §2), so
    // "starting" a recording just means picking where in the buffer to cut
    // from — that's ObsEngine's job once it exists.
    void recordingStarted(RaidEncounter const& encounter, QDateTime const& preRollFrom);

    // Emitted after Config::raidOverrunSeconds have really elapsed past
    // ENCOUNTER_END (or immediately, pre-empted, if a re-pull starts before
    // the overrun finishes — see onLineReceived).
    void recordingStopped(RaidEncounter const& encounter, bool success, QDateTime const& stopTime);

    // Emitted the instant CHALLENGE_MODE_START clears the keystone-level
    // threshold. Mirrors recordingStarted()'s pre-roll semantics.
    void dungeonStarted(DungeonRun const& dungeon, QDateTime const& preRollFrom);

    // Emitted after Config::dungeonOverrunSeconds have elapsed past
    // CHALLENGE_MODE_END (or immediately, pre-empted, if the key is zoned
    // back into before the overrun finishes — see handleChallengeModeStart).
    // durationMs is the log's own in-key duration (arg 4), used later for
    // keystone-upgrade-level calculations; unrelated to stopTime's overrun.
    void dungeonStopped(
        DungeonRun const& dungeon, bool success, int durationMs, QDateTime const& stopTime
    );

    // Emitted on every MAP_CHANGE line -- fires on zoning into/out of any
    // area, not just raids/dungeons. Consumers that only care about
    // instanced content filter that themselves.
    void zoneChanged(int mapId, QString const& zoneName);

  private Q_SLOTS:
    void onOverrunElapsed();
    void onDungeonOverrunElapsed();

  private:
    void handleEncounterStart(LogLine const& line);
    void handleEncounterEnd(LogLine const& line);
    void finishPendingStop();
    void handleChallengeModeStart(LogLine const& line);
    void handleChallengeModeEnd(LogLine const& line);
    void finishPendingDungeonStop();

    Config m_config;
    bool m_active = false;
    RaidEncounter m_current;
    bool m_pendingSuccess = false;
    QDateTime m_pendingStopTime;
    QTimer m_overrunTimer;

    bool m_dungeonActive = false;
    DungeonRun m_currentDungeon;
    bool m_pendingDungeonSuccess = false;
    int m_pendingDungeonDurationMs = 0;
    QDateTime m_pendingDungeonStopTime;
    QTimer m_dungeonOverrunTimer;
};
