#pragma once

#include <QObject>
#include <QString>

#include "GameState.h"
#include "LogWatcher.h"
#include "RecordingController.h"

// Owns the daemon's observable state plus the actual detection pipeline:
// LogWatcher tails the combat log, RecordingController turns lines into
// start/stop decisions, and this class fills the shared GameState fact bags
// (ADR-012) that ObserverAdaptor publishes over DBus and PresencePublisher
// projects into Discord Rich Presence. Still no libobs/ObsEngine or
// MetadataStore — this is detection only, per HANDOFF.md roadmap item 5.
class ObserverService : public QObject
{
    Q_OBJECT

  public:
    explicit ObserverService(QString logDirectory, QObject* parent = nullptr);

    // Starts the underlying LogWatcher. Returns false on failure (bad
    // directory, inotify setup failure, etc.) — main.cpp treats that as
    // fatal.
    bool start();

    GameState& gameState();

  private:
    static QString raidDifficultyDisplayName(int difficultyId);
    static QVariantMap encounterBag(RecordingController::RaidEncounter const& encounter);
    static QVariantMap dungeonBag(RecordingController::DungeonRun const& dungeon);

    QString m_logDirectory;
    GameState m_gameState;

    LogWatcher m_watcher;
    RecordingController m_controller;
};
