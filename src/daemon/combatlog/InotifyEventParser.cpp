#include "InotifyEventParser.h"

#include <cstring>

#include <sys/inotify.h>

namespace constellar::inotify
{

namespace
{

constexpr std::size_t kEventHeaderSize = sizeof(struct inotify_event);

}  // namespace

std::optional<std::pair<ParsedEvent, std::span<std::byte const>>> nextEvent(
    std::span<std::byte const> data
)
{
    if (data.size() < kEventHeaderSize)
        return std::nullopt;

    struct inotify_event header;
    std::memcpy(&header, data.data(), kEventHeaderSize);

    std::size_t const totalSize = kEventHeaderSize + header.len;
    if (data.size() < totalSize)
        return std::nullopt;

    ParsedEvent parsed;
    parsed.mask = header.mask;

    if (header.len > 0)
    {
        auto const* namePtr = reinterpret_cast<char const*>(data.data() + kEventHeaderSize);
        std::size_t const nameLen = ::strnlen(namePtr, header.len);
        parsed.name = QString::fromLocal8Bit(namePtr, static_cast<qsizetype>(nameLen));
    }

    return std::make_pair(std::move(parsed), data.subspan(totalSize));
}

}  // namespace constellar::inotify
