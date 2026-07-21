#include "InotifyWatcher.h"

#include <sys/inotify.h>
#include <unistd.h>

#include <cstddef>
#include <span>

#include <QByteArray>
#include <QSocketNotifier>

#include "InotifyEventParser.h"

namespace constellar::inotify
{

namespace
{

constexpr std::size_t kBufferLen = 1024 * (sizeof(struct inotify_event) + 16);

}  // namespace

InotifyWatcher::InotifyWatcher(QObject* parent) : QObject(parent) {}

InotifyWatcher::~InotifyWatcher()
{
    stop();
}

bool InotifyWatcher::start(QString const& directory)
{
    if (isRunning())
        return true;

    m_fd = inotify_init1(IN_NONBLOCK);
    if (m_fd < 0)
        return false;

    QByteArray const dirPath = directory.toLocal8Bit();
    int const wd = inotify_add_watch(
        m_fd, dirPath.constData(), IN_CREATE | IN_MODIFY | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE
    );

    if (wd < 0)
    {
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &InotifyWatcher::onReadyRead);

    return true;
}

void InotifyWatcher::stop()
{
    if (m_notifier)
    {
        m_notifier->setEnabled(false);
        m_notifier->deleteLater();
        m_notifier = nullptr;
    }
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool InotifyWatcher::isRunning() const
{
    return m_fd >= 0;
}

void InotifyWatcher::onReadyRead()
{
    std::vector<WatchEvent> pending;

    alignas(struct inotify_event) std::byte buffer[kBufferLen];

    while (true)
    {
        ssize_t const len = ::read(m_fd, buffer, sizeof(buffer));
        if (len <= 0)
            break;  // EAGAIN (no more events), EOF or a transient error

        std::span<std::byte const> remaining(buffer, static_cast<size_t>(len));
        while (!remaining.empty())
        {
            auto parsed = nextEvent(remaining);
            if (!parsed)
                break;  // drop everything after a bad event

            ParsedEvent const& raw = parsed->first;
            remaining = parsed->second;

            if (raw.name.isEmpty())
                continue;  // self-events on the watched directory

            WatchEvent event;
            event.name = raw.name;
            event.created = (raw.mask & (IN_CREATE | IN_MOVED_TO)) != 0;
            event.deleted = (raw.mask & (IN_DELETE | IN_MOVED_FROM)) != 0;
            event.modified = (raw.mask & IN_MODIFY) != 0;
            pending.emplace_back(std::move(event));
        }
    }

    if (!pending.empty())
        Q_EMIT events(pending);
}

}  // namespace constellar::inotify
