add_library(constellar-observer STATIC)
add_library(constellar::observer ALIAS constellar-observer)

target_sources(constellar-observer
  PUBLIC
    FILE_SET HEADERS
    FILES
      ObserverService.h
      ActivityProjection.h
      dbus/ManagedTypes.h
      dbus/ActivityObject.h
      dbus/ObjectTreePublisher.h
      dbus/adaptors/ObserverDBusAdaptor.h
      dbus/adaptors/ActivityDBusAdaptor.h
      dbus/adaptors/ActivityEncounterDBusAdaptor.h
      dbus/adaptors/ActivityDungeonDBusAdaptor.h
      dbus/adaptors/ObjectManagerAdaptor.h
  PRIVATE
    ObserverService.cpp
    ActivityProjection.cpp
    dbus/ManagedTypes.cpp
    dbus/ActivityObject.cpp
    dbus/ObjectTreePublisher.cpp
    dbus/adaptors/ObserverDBusAdaptor.cpp
    dbus/adaptors/ActivityDBusAdaptor.cpp
    dbus/adaptors/ActivityEncounterDBusAdaptor.cpp
    dbus/adaptors/ActivityDungeonDBusAdaptor.cpp
    dbus/adaptors/ObjectManagerAdaptor.cpp
)

target_link_libraries(constellar-observer
  PUBLIC
    Qt6::Core
    Qt6::DBus
    constellar::contract
    constellar::common
    constellar::combatlog
    constellar::tracker
    constellar::gamestate
  PRIVATE
    constellar::activitymetadata
)
