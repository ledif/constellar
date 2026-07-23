add_library(constellar-observer STATIC)
add_library(constellar::observer ALIAS constellar-observer)

qt_add_resources(constellar-observer "observer_dbus_xml"
  PREFIX "/dbus"
  BASE ${CMAKE_SOURCE_DIR}/data
  FILES ${CMAKE_SOURCE_DIR}/data/dev.ulduar.Constellar1.xml
)

target_sources(constellar-observer
  PUBLIC
    FILE_SET HEADERS
    FILES
      ObserverService.h
      ActivityProjection.h
      dbus/ManagedTypes.h
      dbus/InterfaceRegistry.h
      dbus/PropertyTreeObject.h
      dbus/ObjectTreePublisher.h
      dbus/adaptors/ObserverDBusAdaptor.h
      dbus/adaptors/ObjectManagerAdaptor.h
  PRIVATE
    ObserverService.cpp
    ActivityProjection.cpp
    dbus/ManagedTypes.cpp
    dbus/InterfaceRegistry.cpp
    dbus/PropertyTreeObject.cpp
    dbus/ObjectTreePublisher.cpp
    dbus/adaptors/ObserverDBusAdaptor.cpp
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
