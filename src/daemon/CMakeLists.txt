add_subdirectory(combatlog)
add_subdirectory(tracker)
add_subdirectory(presence)
add_subdirectory(gamestate)
add_subdirectory(activitymetadata)

add_executable(constellard
  main.cpp
  ObserverDBusAdaptor.h
  ObserverDBusAdaptor.cpp
  ObserverService.h
  ObserverService.cpp
)

target_link_libraries(constellard
  PRIVATE
    Qt6::Core
    Qt6::DBus
    constellar::common
    constellar::combatlog
    constellar::tracker
    constellar::activitymetadata
    constellar::presence
    constellar::gamestate
)

install(TARGETS constellard)
