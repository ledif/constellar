#pragma once

#define CONSTELLAR_DBUS_INTERFACE_NAME "dev.ulduar.Constellar.Observer"

namespace constellar::dbus
{

inline constexpr auto kServiceName = "dev.ulduar.Constellar";
inline constexpr auto kObjectPath = "/dev/ulduar/Constellar/Observer";
inline constexpr auto kInterfaceName = CONSTELLAR_DBUS_INTERFACE_NAME;

}  // namespace constellar::dbus
