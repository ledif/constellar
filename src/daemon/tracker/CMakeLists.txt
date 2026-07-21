add_library(constellar-tracker STATIC)
add_library(constellar::tracker ALIAS constellar-tracker)

target_sources(constellar-tracker
  PUBLIC
    FILE_SET HEADERS
    FILES
      ActivityTracker.h
      DungeonRun.h
      RaidEncounter.h
  PRIVATE
    ActivityTracker.cpp
    RaidEncounter.cpp
)

target_link_libraries(constellar-tracker
  PUBLIC
    Qt6::Core
    constellar::combatlog
    constellar::common
)
