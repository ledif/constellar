add_subdirectory(combatlog)
add_subdirectory(tracker)
add_subdirectory(presence)
add_subdirectory(gamestate)
add_subdirectory(activitymetadata)
add_subdirectory(observer)

add_executable(constellard main.cpp)

target_link_libraries(constellard
  PRIVATE
    Qt6::Core
    Qt6::DBus
    constellar::common
    constellar::observer
    constellar::presence
    constellar::gamestate
)

install(TARGETS constellard)
