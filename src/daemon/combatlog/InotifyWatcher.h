#pragma once

#include <vector>

#include <QObject>
#include <QString>

class QSocketNotifier;

namespace constellar::inotify
{

// A single filesystem change reported for the watched directory. `created`
// covers both a new file and one moved in; `deleted` covers both removal
// and one moved out.
struct WatchEvent
{
    bool created = false;
    bool deleted = false;
    bool modified = false;
    QString name;
};

// Watches one directory (non-recursively) for file create/delete/modify
// activity, and emits the resulting events. sys/inotify.h and the fd/event
// loop plumbing are implementation details confined to InotifyWatcher.cpp
// and InotifyEventParser.cpp — callers only ever see WatchEvent.
class InotifyWatcher : public QObject
{
    Q_OBJECT

  public:
    explicit InotifyWatcher(QObject* parent = nullptr);
    ~InotifyWatcher() override;

    // Starts watching `directory`. Returns false, leaving the watcher not
    // running, if the underlying inotify calls fail.
    bool start(QString const& directory);
    void stop();
    bool isRunning() const;

  Q_SIGNALS:
    void events(std::vector<WatchEvent> const& events);

  private Q_SLOTS:
    void onReadyRead();

  private:
    int m_fd = -1;
    QSocketNotifier* m_notifier = nullptr;
};

}  // namespace constellar::inotify
