#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <utility>

#include <QString>

namespace constellar::inotify
{

struct ParsedEvent
{
    std::uint32_t mask = 0;
    QString name;
};

// parse one inotify_event off the front of a span of bytes
std::optional<std::pair<ParsedEvent, std::span<std::byte const>>> nextEvent(
    std::span<std::byte const> data
);

}  // namespace constellar::inotify
