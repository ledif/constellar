#pragma once

#include <filesystem>

#include <QHash>
#include <QObject>
#include <QString>
#include <QTimer>

#include "InotifyWatcher.h"
#include "LogLine.h"

class QSocketNotifier;

class LogWatcher : public QObject
{
    Q_OBJECT

  public:
    explicit LogWatcher(
        std::filesystem::path directory, int idleTimeoutMs = 60'000, QObject* parent = nullptr
    );

    ~LogWatcher() override;

    bool start();
    void stop();

    bool isRunning() const
    {
        return m_inotifyWatcher.isRunning();
    }

  Q_SIGNALS:
    void lineReceived(LogLine const& line);
    void idleTimeout();

  private Q_SLOTS:
    void onInotifyReadyRead();
    void onIdleTimer();

  private:
    struct WatchedFile
    {
        qint64 offset = 0;
        QByteArray pendingPartial;
    };

    void scanExistingFiles();
    void handleCreateOrMove(QString const& fileName);
    void handleDelete(QString const& fileName);
    void handleModify(QString const& fileName);
    void readNewData(QString const& fileName, WatchedFile& file);
    void resetIdleTimer();
    static bool isCombatLogName(QString const& fileName);

    QString m_directory;
    constellar::inotify::Watcher m_inotifyWatcher;
    QSocketNotifier* m_notifier = nullptr;
    QTimer m_idleTimer;
    QHash<QString, WatchedFile> m_files;
};
