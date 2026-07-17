add_library(constellar-log STATIC)
add_library(constellar::log ALIAS constellar-log)

target_sources(constellar-log
  PUBLIC
    FILE_SET HEADERS
    FILES
      LogLine.h
      LogWatcher.h
  PRIVATE
    LogLine.cpp
    LogWatcher.cpp
)

target_link_libraries(constellar-log PUBLIC Qt6::Core)

