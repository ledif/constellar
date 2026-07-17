#include "LogWatcher.h"

#include <sys/inotify.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSocketNotifier>

using namespace Qt::StringLiterals;

namespace
{
constexpr size_t kInotifyEventSize = sizeof(struct inotify_event);
constexpr size_t kInotifyBufferLen = 1024 * (kInotifyEventSize + 16);
}  // namespace

LogWatcher::LogWatcher(std::filesystem::path directory, int idleTimeoutMs, QObject* parent)
    : QObject(parent),
      m_directory(QString::fromStdString(directory.string())),
      m_idleTimeoutMs(idleTimeoutMs)
{
    m_idleTimer.setSingleShot(true);
    connect(&m_idleTimer, &QTimer::timeout, this, &LogWatcher::onIdleTimer);
}

LogWatcher::~LogWatcher()
{
    stop();
}

bool LogWatcher::isCombatLogName(QString const& fileName)
{
    return fileName.startsWith(u"WoWCombatLog"_s) && fileName.endsWith(u".txt"_s);
}

bool LogWatcher::start()
{
    if (isRunning())
        return true;

    m_inotifyFd = inotify_init1(IN_NONBLOCK);
    if (m_inotifyFd < 0)
        return false;

    QByteArray const dirPath = m_directory.toLocal8Bit();
    m_dirWatchDescriptor = inotify_add_watch(
        m_inotifyFd, dirPath.constData(),
        IN_CREATE | IN_MODIFY | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE
    );
    if (m_dirWatchDescriptor < 0)
    {
        ::close(m_inotifyFd);
        m_inotifyFd = -1;
        return false;
    }

    m_notifier = new QSocketNotifier(m_inotifyFd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &LogWatcher::onInotifyReadyRead);

    scanExistingFiles();
    return true;
}

void LogWatcher::stop()
{
    if (m_notifier)
    {
        m_notifier->setEnabled(false);
        m_notifier->deleteLater();
        m_notifier = nullptr;
    }
    if (m_inotifyFd >= 0)
    {
        ::close(m_inotifyFd);
        m_inotifyFd = -1;
    }
    m_dirWatchDescriptor = -1;
    m_files.clear();
    m_idleTimer.stop();
}

void LogWatcher::scanExistingFiles()
{
    QDir const dir(m_directory);
    QStringList const entries = dir.entryList(QDir::Files);
    for (QString const& name : entries)
    {
        if (!isCombatLogName(name))
            continue;
        // Tail from the current end: a daemon (re)start should not replay a
        // combat log that's already megabytes long.
        WatchedFile file;
        file.offset = QFileInfo(dir.filePath(name)).size();
        m_files.insert(name, file);
    }
}

void LogWatcher::handleCreateOrMove(QString const& fileName)
{
    if (!isCombatLogName(fileName))
        return;
    // Drop any prior state so the next write is read from the start — this
    // is what makes a file recreated with the same name (log rotation) read
    // from byte 0 instead of an offset from before it existed.
    m_files.remove(fileName);
}

void LogWatcher::handleDelete(QString const& fileName)
{
    if (!isCombatLogName(fileName))
        return;
    m_files.remove(fileName);
}

void LogWatcher::handleModify(QString const& fileName)
{
    if (!isCombatLogName(fileName))
        return;
    WatchedFile& file = m_files[fileName];
    readNewData(fileName, file);
    resetIdleTimer();
}

void LogWatcher::readNewData(QString const& fileName, WatchedFile& file)
{
    QFile f(QDir(m_directory).filePath(fileName));
    if (!f.open(QIODevice::ReadOnly))
        return;

    qint64 const size = f.size();
    if (size < file.offset)
    {
        // File was truncated (or replaced without a create/delete event
        // reaching us) — restart from the beginning.
        file.offset = 0;
        file.pendingPartial.clear();
    }
    if (size == file.offset)
        return;

    f.seek(file.offset);
    QByteArray const chunk = f.readAll();
    file.offset = f.pos();

    QByteArray const combined = file.pendingPartial + chunk;
    QList<QByteArray> parts = combined.split('\n');
    file.pendingPartial = parts.isEmpty() ? QByteArray() : parts.takeLast();

    for (QByteArray const& part : std::as_const(parts))
    {
        if (part.isEmpty())
            continue;
        LogLine line(QString::fromUtf8(part));
        if (line.isValid())
            Q_EMIT lineReceived(line);
    }
}

void LogWatcher::resetIdleTimer()
{
    if (m_idleTimeoutMs > 0)
        m_idleTimer.start(m_idleTimeoutMs);
}

void LogWatcher::onIdleTimer()
{
    Q_EMIT idleTimeout();
}

void LogWatcher::onInotifyReadyRead()
{
    char buffer[kInotifyBufferLen];

    while (true)
    {
        ssize_t const len = ::read(m_inotifyFd, buffer, sizeof(buffer));
        if (len < 0)
            break;  // EAGAIN (no more events) or a transient error either way.
        if (len == 0)
            break;

        ssize_t i = 0;
        while (i < len)
        {
            auto const* event = reinterpret_cast<const struct inotify_event*>(buffer + i);
            if (event->len > 0)
            {
                QString const name = QString::fromLocal8Bit(event->name);
                if (event->mask & (IN_CREATE | IN_MOVED_TO))
                    handleCreateOrMove(name);
                if (event->mask & (IN_DELETE | IN_MOVED_FROM))
                    handleDelete(name);
                if (event->mask & IN_MODIFY)
                    handleModify(name);
            }
            i += static_cast<ssize_t>(kInotifyEventSize + event->len);
        }
    }
}
