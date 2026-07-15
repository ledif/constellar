#pragma once

#include <QObject>
#include <QString>

#include "LogWatcher.h"
#include "RecordingController.h"

// Owns the daemon's observable state plus the actual detection pipeline:
// LogWatcher tails the combat log, RecordingController turns lines into
// start/stop decisions, and this class translates those decisions into the
// primitive-typed Qt signals ManagerAdaptor relays over DBus (see PLAN.md
// §3.7). Still no libobs/ObsEngine or MetadataStore — this is detection
// only, per HANDOFF.md roadmap item 5.
class ManagerService : public QObject {
    Q_OBJECT

  public:
    explicit ManagerService(QString logDirectory, QObject *parent = nullptr);

    // Starts the underlying LogWatcher. Returns false on failure (bad
    // directory, inotify setup failure, etc.) — main.cpp treats that as
    // fatal, same posture as tools/logtail.
    bool start();

    QString state() const;
    bool wowActive() const;
    QString activeCapture() const;

  Q_SIGNALS:
    void stateChanged(const QString &state);

    // Mirror RecordingController's decisions with primitive-typed
    // signals so ManagerAdaptor can relay them verbatim over DBus (whose
    // marshalling doesn't know about RaidEncounter/DungeonRun or
    // QDateTime). Timestamps are ISO 8601 strings.
    void encounterDetected(int encounterId, const QString &encounterName, const QString &difficulty,
                           const QString &startTime);
    void encounterEnded(int encounterId, const QString &encounterName, bool success,
                        const QString &stopTime);
    void dungeonDetected(int zoneId, int mapId, int keystoneLevel, const QString &startTime);
    void dungeonEnded(int mapId, int keystoneLevel, bool success, int durationMs,
                      const QString &stopTime);

  private:
    void setState(const QString &state);
    static QString raidDifficultyDisplayName(int difficultyId);

    QString m_logDirectory;
    QString m_state = QStringLiteral("idle");
    bool m_wowActive = false;
    QString m_activeCapture = QStringLiteral("none");

    LogWatcher m_watcher;
    RecordingController m_controller;
};
