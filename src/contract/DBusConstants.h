#pragma once

#include <QString>

// These macros exist because of Q_CLASSINFO("D-Bus Interface", ...)
#define CONSTELLAR_DBUS_INTERFACE_NAME "dev.ulduar.Constellar1.Observer"
#define CONSTELLAR_DBUS_ACTIVITY_INTERFACE_NAME "dev.ulduar.Constellar1.Activity"
#define CONSTELLAR_DBUS_ACTIVITY_ENCOUNTER_INTERFACE_NAME \
    "dev.ulduar.Constellar1.Activity.Encounter"
#define CONSTELLAR_DBUS_ACTIVITY_DUNGEON_INTERFACE_NAME "dev.ulduar.Constellar1.Activity.Dungeon"
#define CONSTELLAR_DBUS_OBJECT_MANAGER_INTERFACE_NAME "org.freedesktop.DBus.ObjectManager"

namespace constellar::dbus
{

using namespace Qt::Literals::StringLiterals;

inline QString const kServiceName = u"dev.ulduar.Constellar1"_s;
inline QString const kObjectPath = u"/dev/ulduar/Constellar1/Observer"_s;

inline QString const kRootObjectPath = u"/dev/ulduar/Constellar1"_s;
inline QString const kActivityObjectPath = u"/dev/ulduar/Constellar1/activity/current"_s;

inline QString const kInterfaceName = u"" CONSTELLAR_DBUS_INTERFACE_NAME ""_s;

inline QString const kActivityInterfaceName = u"" CONSTELLAR_DBUS_ACTIVITY_INTERFACE_NAME ""_s;

inline QString const kActivityEncounterInterfaceName =
    u"" CONSTELLAR_DBUS_ACTIVITY_ENCOUNTER_INTERFACE_NAME ""_s;

inline QString const kActivityDungeonInterfaceName =
    u"" CONSTELLAR_DBUS_ACTIVITY_DUNGEON_INTERFACE_NAME ""_s;

inline QString const kObjectManagerInterfaceName =
    u"" CONSTELLAR_DBUS_OBJECT_MANAGER_INTERFACE_NAME ""_s;

}  // namespace constellar::dbus
