#pragma once

#define CONSTELLAR_DBUS_INTERFACE_NAME "dev.ulduar.Constellar1.Observer"

namespace constellar::dbus
{

inline constexpr auto kServiceName = "dev.ulduar.Constellar1";
inline constexpr auto kObjectPath = "/dev/ulduar/Constellar1/Observer";
inline constexpr auto kInterfaceName = CONSTELLAR_DBUS_INTERFACE_NAME;

}  // namespace constellar::dbus
