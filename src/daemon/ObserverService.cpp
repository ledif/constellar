#include "ObserverService.h"

#include <QDateTime>

ObserverService::ObserverService(QString logDirectory, QObject *parent)
    : QObject(parent),
      m_logDirectory(std::move(logDirectory)),
      m_watcher(m_logDirectory),
      m_controller(RecordingController::Config{}) {
    connect(&m_watcher, &LogWatcher::lineReceived, &m_controller,
            &RecordingController::onLineReceived);

    connect(&m_watcher, &LogWatcher::idleTimeout, this, [this]() {
        m_wowActive = false;
        setState(QStringLiteral("idle"));
    });

    connect(&m_watcher, &LogWatcher::lineReceived, this, [this](const LogLine & /*line*/) {
        if (!m_wowActive) {
            m_wowActive = true;
            setState(QStringLiteral("watching"));
        }
    });

    connect(&m_controller, &RecordingController::recordingStarted, this,
            [this](const RecordingController::RaidEncounter &encounter,
                   const QDateTime & /*preRollFrom*/) {
                setState(QStringLiteral("encounter"));
                Q_EMIT encounterDetected(encounter.encounterId, encounter.encounterName,
                                         raidDifficultyDisplayName(encounter.difficultyId),
                                         encounter.startTime.toString(Qt::ISODateWithMs));
            });
    connect(&m_controller, &RecordingController::recordingStopped, this,
            [this](const RecordingController::RaidEncounter &encounter, bool success,
                   const QDateTime &stopTime) {
                setState(QStringLiteral("watching"));
                Q_EMIT encounterEnded(encounter.encounterId, encounter.encounterName, success,
                                      stopTime.toString(Qt::ISODateWithMs));
            });
    connect(
        &m_controller, &RecordingController::dungeonStarted, this,
        [this](const RecordingController::DungeonRun &dungeon, const QDateTime & /*preRollFrom*/) {
            setState(QStringLiteral("dungeon"));
            Q_EMIT dungeonDetected(dungeon.zoneId, dungeon.mapId, dungeon.keystoneLevel,
                                   dungeon.startTime.toString(Qt::ISODateWithMs));
        });
    connect(&m_controller, &RecordingController::dungeonStopped, this,
            [this](const RecordingController::DungeonRun &dungeon, bool success, int durationMs,
                   const QDateTime &stopTime) {
                setState(QStringLiteral("watching"));
                Q_EMIT dungeonEnded(dungeon.mapId, dungeon.keystoneLevel, success, durationMs,
                                    stopTime.toString(Qt::ISODateWithMs));
            });
    connect(&m_controller, &RecordingController::zoneChanged, this, &ObserverService::zoneChanged);
}

bool ObserverService::start() {
    return m_watcher.start();
}

QString ObserverService::state() const {
    return m_state;
}

bool ObserverService::wowActive() const {
    return m_wowActive;
}

QString ObserverService::activeCapture() const {
    return m_activeCapture;
}

void ObserverService::setState(const QString &state) {
    if (m_state == state) {
        return;
    }
    m_state = state;
    Q_EMIT stateChanged(m_state);
}

QString ObserverService::raidDifficultyDisplayName(int difficultyId) {
    const std::optional<RecordingController::RaidDifficulty> difficulty =
        RecordingController::raidDifficultyFromId(difficultyId);
    if (!difficulty) {
        return QStringLiteral("Unknown");
    }
    switch (*difficulty) {
        case RecordingController::RaidDifficulty::LFR:
            return QStringLiteral("LFR");
        case RecordingController::RaidDifficulty::Normal:
            return QStringLiteral("Normal");
        case RecordingController::RaidDifficulty::Heroic:
            return QStringLiteral("Heroic");
        case RecordingController::RaidDifficulty::Mythic:
            return QStringLiteral("Mythic");
    }
    return QStringLiteral("Unknown");
}
