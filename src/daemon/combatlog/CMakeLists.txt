add_library(constellar-combatlog STATIC)
add_library(constellar::combatlog ALIAS constellar-combatlog)

target_sources(constellar-combatlog
  PUBLIC
    FILE_SET HEADERS
    FILES
      LogLine.h
      LogWatcher.h
      InotifyEventParser.h
      InotifyWatcher.h
  PRIVATE
    LogLine.cpp
    LogWatcher.cpp
    InotifyEventParser.cpp
    InotifyWatcher.cpp
)

target_link_libraries(constellar-combatlog PUBLIC Qt6::Core)

