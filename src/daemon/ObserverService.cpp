#include "ObserverService.h"

#include <QDateTime>

#include "ActivityKeys.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

ObserverService::ObserverService(std::filesystem::path const& logDirectory, QObject* parent)
    : QObject(parent),
      m_logDirectory(std::move(logDirectory)),
      m_watcher(m_logDirectory),
      m_controller(RecordingController::Config{})
{
    connect(
        &m_watcher, &LogWatcher::lineReceived, &m_controller, &RecordingController::onLineReceived
    );

    connect(
        &m_controller, &RecordingController::recordingStarted, this,
        [this](
            RecordingController::RaidEncounter const& encounter, QDateTime const& /*preRollFrom*/
        ) { m_gameState.setActivity(encounterBag(encounter)); }
    );
    connect(
        &m_controller, &RecordingController::recordingStopped, this,
        [this](
            RecordingController::RaidEncounter const& encounter, bool success,
            QDateTime const& stopTime
        )
        {
            QVariantMap bag = encounterBag(encounter);
            bag[keys::kSuccess] = success;
            bag[keys::kStopTime] = static_cast<qint64>(stopTime.toMSecsSinceEpoch());
            m_gameState.endActivity(bag);
        }
    );
    connect(
        &m_controller, &RecordingController::dungeonStarted, this,
        [this](RecordingController::DungeonRun const& dungeon, QDateTime const& /*preRollFrom*/)
        { m_gameState.setActivity(dungeonBag(dungeon)); }
    );
    connect(
        &m_controller, &RecordingController::dungeonStopped, this,
        [this](
            RecordingController::DungeonRun const& dungeon, bool success, int durationMs,
            QDateTime const& stopTime
        )
        {
            QVariantMap bag = dungeonBag(dungeon);
            bag[keys::kSuccess] = success;
            bag[keys::kDurationMs] = static_cast<qint64>(durationMs);
            bag[keys::kStopTime] = static_cast<qint64>(stopTime.toMSecsSinceEpoch());
            m_gameState.endActivity(bag);
        }
    );
    connect(
        &m_controller, &RecordingController::zoneChanged, this,
        [this](int mapId, QString const& zoneName)
        {
            m_gameState.setZone(
                QVariantMap{{keys::kMapId, static_cast<uint>(mapId)}, {keys::kZoneName, zoneName}}
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

QString ObserverService::raidDifficultyDisplayName(int difficultyId)
{
    std::optional<RecordingController::RaidDifficulty> const difficulty =
        RecordingController::raidDifficultyFromId(difficultyId);
    if (!difficulty)
        return u"Unknown"_s;
    switch (*difficulty)
    {
        case RecordingController::RaidDifficulty::LFR:
            return u"LFR"_s;
        case RecordingController::RaidDifficulty::Normal:
            return u"Normal"_s;
        case RecordingController::RaidDifficulty::Heroic:
            return u"Heroic"_s;
        case RecordingController::RaidDifficulty::Mythic:
            return u"Mythic"_s;
    }
    return u"Unknown"_s;
}

QVariantMap ObserverService::encounterBag(RecordingController::RaidEncounter const& encounter)
{
    QVariantMap bag;
    bag[keys::kType] = QString::fromLatin1(keys::kTypeEncounter);
    bag[keys::kEncounterId] = static_cast<uint>(encounter.encounterId);
    bag[keys::kEncounterName] = encounter.encounterName;
    bag[keys::kDifficulty] = raidDifficultyDisplayName(encounter.difficultyId);
    bag[keys::kDifficultyId] = static_cast<uint>(encounter.difficultyId);
    bag[keys::kStartTime] = static_cast<qint64>(encounter.startTime.toMSecsSinceEpoch());
    return bag;
}

QVariantMap ObserverService::dungeonBag(RecordingController::DungeonRun const& dungeon)
{
    QVariantMap bag;
    bag[keys::kType] = QString::fromLatin1(keys::kTypeDungeon);
    bag[keys::kMapId] = static_cast<uint>(dungeon.mapId);
    bag[keys::kZoneId] = static_cast<uint>(dungeon.zoneId);
    bag[keys::kKeystoneLevel] = static_cast<uint>(dungeon.keystoneLevel);
    bag[keys::kStartTime] = static_cast<qint64>(dungeon.startTime.toMSecsSinceEpoch());
    return bag;
}
