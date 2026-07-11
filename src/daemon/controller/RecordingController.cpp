#include "RecordingController.h"

RecordingController::RecordingController(Config config, QObject *parent)
    : QObject(parent), m_config(std::move(config)) {
    m_overrunTimer.setSingleShot(true);
    connect(&m_overrunTimer, &QTimer::timeout, this, &RecordingController::onOverrunElapsed);
}

std::optional<RecordingController::RaidDifficulty> RecordingController::raidDifficultyFromId(
    int difficultyId) {
    switch (difficultyId) {
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

void RecordingController::onLineReceived(const LogLine &line) {
    if (!line.isValid()) {
        return;
    }

    const QString type = line.type();
    if (type == QStringLiteral("ENCOUNTER_START")) {
        handleEncounterStart(line);
    } else if (type == QStringLiteral("ENCOUNTER_END")) {
        handleEncounterEnd(line);
    }
}

void RecordingController::handleEncounterStart(const LogLine &line) {
    // ENCOUNTER_START args: encounterID, encounterName, difficultyID,
    // groupSize, instanceID (PLAN.md §6).
    if (line.argCount() < 5) {
        return;
    }

    if (m_active) {
        // A new pull always follows an ENCOUNTER_END for the previous one,
        // so if we're still "active" it means the previous encounter's
        // overrun tail hasn't finished yet (a quick re-pull). Cut it short
        // now rather than making the new pull wait — the underlying replay
        // buffer never stops, this only affects where we call the previous
        // segment finished.
        if (m_overrunTimer.isActive()) {
            m_overrunTimer.stop();
            finishPendingStop();
        }
        m_active = false;
    }

    const int difficultyId = line.argString(3).toInt();
    const std::optional<RaidDifficulty> difficulty = raidDifficultyFromId(difficultyId);
    if (!difficulty || *difficulty < m_config.minDifficulty) {
        return;
    }

    m_current = RaidEncounter{
        line.argString(1).toInt(),
        line.argString(2),
        difficultyId,
        line.dateTime(),
    };
    m_active = true;

    const QDateTime preRollFrom = m_current.startTime.addSecs(-m_config.preRollSeconds);
    Q_EMIT recordingStarted(m_current, preRollFrom);
}

void RecordingController::handleEncounterEnd(const LogLine &line) {
    // ENCOUNTER_END args: encounterID, encounterName, difficultyID,
    // groupSize, success(0/1) (PLAN.md §6).
    if (!m_active || line.argCount() < 5) {
        return;
    }

    const int encounterId = line.argString(1).toInt();
    if (encounterId != m_current.encounterId) {
        // Stray END that doesn't match the encounter we're tracking; ignore
        // rather than risk stopping the wrong recording.
        return;
    }

    m_pendingSuccess = line.argString(5) == QStringLiteral("1");
    m_pendingStopTime = line.dateTime().addSecs(m_config.raidOverrunSeconds);
    m_overrunTimer.start(m_config.raidOverrunSeconds * 1000);
}

void RecordingController::onOverrunElapsed() {
    finishPendingStop();
    m_active = false;
}

void RecordingController::finishPendingStop() {
    Q_EMIT recordingStopped(m_current, m_pendingSuccess, m_pendingStopTime);
}
