#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QTimer>

#include "LogLine.h"

class QSocketNotifier;

// Tails WoWCombatLog*.txt files in a directory via inotify, emitting a
// LogLine for every complete line written. Handles partial lines at EOF,
// multiple files in the directory, and file recreation (new key on
// create/delete rather than following renames — see PLAN.md §3.2).
//
// Does not itself decide when a "session" starts/stops; RecordingController
// (not yet implemented) is the consumer that turns idleTimeout()/
// lineReceived() into buffer start/stop decisions.
class LogWatcher : public QObject {
    Q_OBJECT

  public:
    // idleTimeoutMs: emit idleTimeout() after this long without any write to
    // any watched file. 0 disables the idle timer.
    explicit LogWatcher(QString directory, int idleTimeoutMs = 60'000, QObject *parent = nullptr);
    ~LogWatcher() override;

    // Opens the inotify fd, watches the directory, and picks up any
    // already-existing WoWCombatLog*.txt files (tailing from their current
    // end, not replaying old content). Returns false on failure.
    bool start();
    void stop();

    bool isRunning() const {
        return m_inotifyFd >= 0;
    }

  Q_SIGNALS:
    void lineReceived(const LogLine &line);
    void idleTimeout();

  private Q_SLOTS:
    void onInotifyReadyRead();
    void onIdleTimer();

  private:
    struct WatchedFile {
        qint64 offset = 0;
        QByteArray pendingPartial;
    };

    void scanExistingFiles();
    void handleCreateOrMove(const QString &fileName);
    void handleDelete(const QString &fileName);
    void handleModify(const QString &fileName);
    void readNewData(const QString &fileName, WatchedFile &file);
    void resetIdleTimer();
    static bool isCombatLogName(const QString &fileName);

    QString m_directory;
    int m_idleTimeoutMs;
    int m_inotifyFd = -1;
    int m_dirWatchDescriptor = -1;
    QSocketNotifier *m_notifier = nullptr;
    QTimer m_idleTimer;
    QHash<QString, WatchedFile> m_files;
};
