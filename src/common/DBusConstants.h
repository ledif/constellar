#pragma once

#define CONSTELLAR_DBUS_INTERFACE_NAME "io.github.ledif.constellar.Observer"

namespace constellar::dbus
{

inline constexpr auto kServiceName = "io.github.ledif.constellar";
inline constexpr auto kObjectPath = "/io/github/ledif/constellar/Observer";
inline constexpr auto kInterfaceName = CONSTELLAR_DBUS_INTERFACE_NAME;

}  // namespace constellar::dbus
