# Combat-log tailing + parsing. Split into its own static library so tests/ can link it without
# pulling in the daemon's main.cpp.
add_library(constellar-log STATIC LogLine.h LogLine.cpp LogWatcher.h LogWatcher.cpp)
add_library(constellar::log ALIAS constellar-log)

target_include_directories(constellar-log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(constellar-log PUBLIC Qt6::Core)
