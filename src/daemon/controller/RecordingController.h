#pragma once

#include <optional>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

#include "LogLine.h"

// Raids-only first cut of the encounter state machine described in
// PLAN.md §3.4. Consumes parsed LogLines and decides when a raid pull
// should be recorded — it does not touch libobs (that's ObsEngine, Phase 2)
// or SQLite (MetadataStore); it just emits start/stop decisions.
//
// M+ nesting, delves, and ZONE_CHANGE handling are out of scope here; only
// ENCOUNTER_START/ENCOUNTER_END are consumed. Everything else is ignored.
class RecordingController : public QObject {
    Q_OBJECT

  public:
    // Raid difficulties in increasing order of difficulty. WoW's numeric
    // difficultyIDs are NOT ordered this way (LFR=17 > Mythic=16), so we
    // can't threshold on the raw ID — this is a minimal stand-in for the
    // full instanceDifficulty table (PLAN.md §3.5/§6), covering just the
    // four raid difficulties needed to decide record-or-skip.
    enum class RaidDifficulty { LFR = 0, Normal = 1, Heroic = 2, Mythic = 3 };

    struct Config {
        int preRollSeconds = 15;
        int raidOverrunSeconds = 20;
        RaidDifficulty minDifficulty = RaidDifficulty::Normal;
    };

    struct RaidEncounter {
        int encounterId = 0;
        QString encounterName;
        int difficultyId = 0;
        QDateTime startTime;
    };

    explicit RecordingController(Config config, QObject *parent = nullptr);

    // Maps a WoW difficultyID to a RaidDifficulty rank, or nullopt if it's
    // not one of the four raid difficulties (e.g. a dungeon/M+ ID).
    static std::optional<RaidDifficulty> raidDifficultyFromId(int difficultyId);

  public Q_SLOTS:
    void onLineReceived(const LogLine &line);

  Q_SIGNALS:
    // Emitted the instant ENCOUNTER_START clears the difficulty threshold.
    // preRollFrom = startTime - Config::preRollSeconds. This isn't a
    // scheduling delay: the replay buffer runs continuously (PLAN.md §2), so
    // "starting" a recording just means picking where in the buffer to cut
    // from — that's ObsEngine's job once it exists.
    void recordingStarted(const RaidEncounter &encounter, const QDateTime &preRollFrom);

    // Emitted after Config::raidOverrunSeconds have really elapsed past
    // ENCOUNTER_END (or immediately, pre-empted, if a re-pull starts before
    // the overrun finishes — see onLineReceived).
    void recordingStopped(const RaidEncounter &encounter, bool success, const QDateTime &stopTime);

  private Q_SLOTS:
    void onOverrunElapsed();

  private:
    void handleEncounterStart(const LogLine &line);
    void handleEncounterEnd(const LogLine &line);
    void finishPendingStop();

    Config m_config;
    bool m_active = false;
    RaidEncounter m_current;
    bool m_pendingSuccess = false;
    QDateTime m_pendingStopTime;
    QTimer m_overrunTimer;
};
