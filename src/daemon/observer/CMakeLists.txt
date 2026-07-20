add_library(constellar-observer STATIC)
add_library(constellar::observer ALIAS constellar-observer)

target_sources(constellar-observer
  PUBLIC
    FILE_SET HEADERS
    FILES
      ObserverService.h
      ObserverDBusAdaptor.h
  PRIVATE
    ObserverService.cpp
    ObserverDBusAdaptor.cpp
)

target_link_libraries(constellar-observer
  PUBLIC
    Qt6::Core
    Qt6::DBus
    constellar::common
    constellar::combatlog
    constellar::tracker
    constellar::gamestate
  PRIVATE
    constellar::activitymetadata
)
