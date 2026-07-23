#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

#include "ActivityOutcome.h"
#include "DungeonRun.h"
#include "Location.h"
#include "LogLine.h"
#include "RaidEncounter.h"

class ActivityTracker : public QObject
{
    Q_OBJECT

  public:
    struct Config
    {
        int preRollSeconds = 15;
        int raidOverrunSeconds = 20;
        RaidDifficulty minDifficulty = RaidDifficulty::Normal;
        int dungeonOverrunSeconds = 5;
        int minKeystoneLevel = 2;
    };

    explicit ActivityTracker(Config config, QObject* parent = nullptr);

  public Q_SLOTS:
    void onLineReceived(LogLine const& line);

  Q_SIGNALS:
    void encounterStarted(RaidEncounter const& encounter, QDateTime const& preRollFrom);
    void encounterStopped(
        RaidEncounter const& encounter, ActivityOutcome outcome, int durationMs,
        QDateTime const& stopTime
    );

    void dungeonStarted(DungeonRun const& dungeon, QDateTime const& preRollFrom);
    void dungeonStopped(
        DungeonRun const& dungeon, ActivityOutcome outcome, int durationMs,
        QDateTime const& stopTime
    );

    void uiMapChanged(UiMap const& uiMap);
    void zoneChanged(Zone const& zone);

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
    ActivityOutcome m_pendingOutcome = ActivityOutcome::Unknown;
    int m_pendingDurationMs = 0;
    QDateTime m_pendingStopTime;
    QTimer m_overrunTimer;

    bool m_dungeonActive = false;
    DungeonRun m_currentDungeon;
    ActivityOutcome m_pendingDungeonOutcome = ActivityOutcome::Unknown;
    int m_pendingDungeonDurationMs = 0;
    QDateTime m_pendingDungeonStopTime;
    QTimer m_dungeonOverrunTimer;
};
