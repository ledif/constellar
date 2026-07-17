#pragma once

#include <vector>

#include <QString>

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
// activity. sys/inotify.h is an implementation detail confined to
// InotifyWatcher.cpp and InotifyEventParser.cpp — callers only ever see
// WatchEvent.
class Watcher
{
  public:
    ~Watcher();

    // Starts watching `directory`. Returns false, leaving the watcher not
    // running, if the underlying inotify calls fail.
    bool start(QString const& directory);
    void stop();
    bool isRunning() const;

    // The underlying fd, suitable for a QSocketNotifier. -1 if not running.
    int fd() const;

    // Reads and parses all currently-pending events. Non-blocking; call
    // when the fd reports readable.
    std::vector<WatchEvent> readEvents();

  private:
    int m_fd = -1;
    int m_watchDescriptor = -1;
};

}  // namespace constellar::inotify
