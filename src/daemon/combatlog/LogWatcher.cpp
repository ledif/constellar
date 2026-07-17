#include "LogWatcher.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

using namespace Qt::StringLiterals;

LogWatcher::LogWatcher(std::filesystem::path directory, int idleTimeoutMs, QObject* parent)
    : QObject(parent), m_directory(QString::fromStdString(directory.string()))
{
    m_idleTimer.setSingleShot(true);
    m_idleTimer.setInterval(idleTimeoutMs);
    connect(&m_idleTimer, &QTimer::timeout, this, &LogWatcher::onIdleTimer);
    connect(
        &m_inotifyWatcher, &constellar::inotify::InotifyWatcher::events, this,
        &LogWatcher::onInotifyEvents
    );
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

    if (!m_inotifyWatcher.start(m_directory))
        return false;

    scanExistingFiles();
    return true;
}

void LogWatcher::stop()
{
    m_inotifyWatcher.stop();
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
    if (m_idleTimer.interval() > 0)
        m_idleTimer.start();
}

void LogWatcher::onIdleTimer()
{
    Q_EMIT idleTimeout();
}

void LogWatcher::onInotifyEvents(std::vector<constellar::inotify::WatchEvent> const& events)
{
    for (constellar::inotify::WatchEvent const& event : events)
    {
        if (event.created)
            handleCreateOrMove(event.name);
        if (event.deleted)
            handleDelete(event.name);
        if (event.modified)
            handleModify(event.name);
    }
}
