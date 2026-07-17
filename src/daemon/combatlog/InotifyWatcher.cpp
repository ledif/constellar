#include "InotifyWatcher.h"

#include <sys/inotify.h>
#include <unistd.h>

#include <cstddef>
#include <span>

#include <QByteArray>

#include "InotifyEventParser.h"

namespace constellar::inotify
{

namespace
{

constexpr std::size_t kBufferLen = 1024 * (sizeof(struct inotify_event) + 16);

}  // namespace

Watcher::~Watcher()
{
    stop();
}

bool Watcher::start(QString const& directory)
{
    if (isRunning())
        return true;

    m_fd = inotify_init1(IN_NONBLOCK);
    if (m_fd < 0)
        return false;

    QByteArray const dirPath = directory.toLocal8Bit();
    m_watchDescriptor = inotify_add_watch(
        m_fd, dirPath.constData(), IN_CREATE | IN_MODIFY | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE
    );
    if (m_watchDescriptor < 0)
    {
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    return true;
}

void Watcher::stop()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
    m_watchDescriptor = -1;
}

bool Watcher::isRunning() const
{
    return m_fd >= 0;
}

int Watcher::fd() const
{
    return m_fd;
}

std::vector<WatchEvent> Watcher::readEvents()
{
    std::vector<WatchEvent> events;

    alignas(struct inotify_event) std::byte buffer[kBufferLen];

    while (true)
    {
        ssize_t const len = ::read(m_fd, buffer, sizeof(buffer));
        if (len <= 0)
            break;  // EAGAIN (no more events), EOF, or a transient error either way.

        std::span<std::byte const> remaining(buffer, static_cast<size_t>(len));
        while (!remaining.empty())
        {
            auto parsed = nextEvent(remaining);
            if (!parsed)
                break;  // Truncated/malformed trailing event; drop the rest of this read.

            ParsedEvent const& raw = parsed->first;
            remaining = parsed->second;

            if (raw.name.isEmpty())
                continue;  // Self-events on the watched directory itself.

            WatchEvent event;
            event.name = raw.name;
            event.created = (raw.mask & (IN_CREATE | IN_MOVED_TO)) != 0;
            event.deleted = (raw.mask & (IN_DELETE | IN_MOVED_FROM)) != 0;
            event.modified = (raw.mask & IN_MODIFY) != 0;
            events.push_back(std::move(event));
        }
    }

    return events;
}

}  // namespace constellar::inotify
