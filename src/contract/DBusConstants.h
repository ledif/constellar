#pragma once

#define CONSTELLAR_DBUS_INTERFACE_NAME "dev.ulduar.Constellar1.Observer"
#define CONSTELLAR_ACTIVITY_INTERFACE_NAME "dev.ulduar.Constellar1.Activity"
#define CONSTELLAR_ACTIVITY_ENCOUNTER_INTERFACE_NAME "dev.ulduar.Constellar1.Activity.Encounter"
#define CONSTELLAR_ACTIVITY_DUNGEON_INTERFACE_NAME "dev.ulduar.Constellar1.Activity.Dungeon"
#define CONSTELLAR_OBJECT_MANAGER_INTERFACE_NAME "org.freedesktop.DBus.ObjectManager"

namespace constellar::dbus
{

inline constexpr auto kServiceName = "dev.ulduar.Constellar1";
inline constexpr auto kObjectPath = "/dev/ulduar/Constellar1/Observer";
inline constexpr auto kInterfaceName = CONSTELLAR_DBUS_INTERFACE_NAME;

// RFC-006 object-path contract (TASK-008/TASK-009). The root hosts the standard
// ObjectManager; /activity/current is registered only while an activity is live.
inline constexpr auto kRootObjectPath = "/dev/ulduar/Constellar1";
inline constexpr auto kActivityObjectPath = "/dev/ulduar/Constellar1/activity/current";
inline constexpr auto kActivityInterfaceName = CONSTELLAR_ACTIVITY_INTERFACE_NAME;
inline constexpr auto kActivityEncounterInterfaceName =
    CONSTELLAR_ACTIVITY_ENCOUNTER_INTERFACE_NAME;
inline constexpr auto kActivityDungeonInterfaceName = CONSTELLAR_ACTIVITY_DUNGEON_INTERFACE_NAME;
inline constexpr auto kObjectManagerInterfaceName = CONSTELLAR_OBJECT_MANAGER_INTERFACE_NAME;

}  // namespace constellar::dbus
