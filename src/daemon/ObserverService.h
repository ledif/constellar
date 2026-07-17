#pragma once

#include <filesystem>

#include <QObject>
#include <QString>

#include "ActivityTracker.h"
#include "GameState.h"
#include "LogWatcher.h"

class ObserverService : public QObject
{
    Q_OBJECT

  public:
    explicit ObserverService(std::filesystem::path const& logDirectory, QObject* parent = nullptr);

    bool start();

    GameState const& gameState() const;

  private:
    std::filesystem::path m_logDirectory;
    GameState m_gameState;

    LogWatcher m_watcher;
    ActivityTracker m_tracker;
};
