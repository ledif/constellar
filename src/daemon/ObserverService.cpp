#include "ObserverService.h"

#include <QDateTime>

#include "ActivityKeys.h"

namespace keys = constellar::keys;

ObserverService::ObserverService(QString logDirectory, QObject *parent)
    : QObject(parent),
      m_logDirectory(std::move(logDirectory)),
      m_watcher(m_logDirectory),
      m_controller(RecordingController::Config{}) {
    connect(&m_watcher, &LogWatcher::lineReceived, &m_controller,
            &RecordingController::onLineReceived);

    connect(&m_controller, &RecordingController::recordingStarted, this,
            [this](const RecordingController::RaidEncounter &encounter,
                   const QDateTime & /*preRollFrom*/) {
                m_gameState.setActivity(encounterBag(encounter));
            });
    connect(&m_controller, &RecordingController::recordingStopped, this,
            [this](const RecordingController::RaidEncounter &encounter, bool success,
                   const QDateTime &stopTime) {
                QVariantMap bag = encounterBag(encounter);
                bag[keys::kSuccess] = success;
                bag[keys::kStopTime] = static_cast<qint64>(stopTime.toMSecsSinceEpoch());
                m_gameState.endActivity(bag);
            });
    connect(
        &m_controller, &RecordingController::dungeonStarted, this,
        [this](const RecordingController::DungeonRun &dungeon, const QDateTime & /*preRollFrom*/) {
            m_gameState.setActivity(dungeonBag(dungeon));
        });
    connect(&m_controller, &RecordingController::dungeonStopped, this,
            [this](const RecordingController::DungeonRun &dungeon, bool success, int durationMs,
                   const QDateTime &stopTime) {
                QVariantMap bag = dungeonBag(dungeon);
                bag[keys::kSuccess] = success;
                bag[keys::kDurationMs] = static_cast<qint64>(durationMs);
                bag[keys::kStopTime] = static_cast<qint64>(stopTime.toMSecsSinceEpoch());
                m_gameState.endActivity(bag);
            });
    connect(&m_controller, &RecordingController::zoneChanged, this,
            [this](int mapId, const QString &zoneName) {
                m_gameState.setZone(QVariantMap{{keys::kMapId, static_cast<uint>(mapId)},
                                                {keys::kZoneName, zoneName}});
            });
}

bool ObserverService::start() {
    return m_watcher.start();
}

GameState &ObserverService::gameState() {
    return m_gameState;
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

QVariantMap ObserverService::encounterBag(const RecordingController::RaidEncounter &encounter) {
    QVariantMap bag;
    bag[keys::kType] = QString::fromLatin1(keys::kTypeEncounter);
    bag[keys::kEncounterId] = static_cast<uint>(encounter.encounterId);
    bag[keys::kEncounterName] = encounter.encounterName;
    bag[keys::kDifficulty] = raidDifficultyDisplayName(encounter.difficultyId);
    bag[keys::kDifficultyId] = static_cast<uint>(encounter.difficultyId);
    bag[keys::kStartTime] = static_cast<qint64>(encounter.startTime.toMSecsSinceEpoch());
    return bag;
}

QVariantMap ObserverService::dungeonBag(const RecordingController::DungeonRun &dungeon) {
    QVariantMap bag;
    bag[keys::kType] = QString::fromLatin1(keys::kTypeDungeon);
    bag[keys::kMapId] = static_cast<uint>(dungeon.mapId);
    bag[keys::kZoneId] = static_cast<uint>(dungeon.zoneId);
    bag[keys::kKeystoneLevel] = static_cast<uint>(dungeon.keystoneLevel);
    bag[keys::kStartTime] = static_cast<qint64>(dungeon.startTime.toMSecsSinceEpoch());
    return bag;
}
