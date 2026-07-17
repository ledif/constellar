#pragma once

#include <filesystem>

#include <QObject>
#include <QString>

#include "GameState.h"
#include "LogWatcher.h"
#include "RecordingController.h"

class ObserverService : public QObject
{
    Q_OBJECT

  public:
    explicit ObserverService(std::filesystem::path const& logDirectory, QObject* parent = nullptr);

    bool start();

    GameState const& gameState() const;

  private:
    static QString raidDifficultyDisplayName(int difficultyId);
    static QVariantMap encounterBag(RecordingController::RaidEncounter const& encounter);
    static QVariantMap dungeonBag(RecordingController::DungeonRun const& dungeon);

    std::filesystem::path m_logDirectory;
    GameState m_gameState;

    LogWatcher m_watcher;
    RecordingController m_controller;
};
