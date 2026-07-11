# Combat-log tailing + parsing. Split into its own static library so
# tests/ can link it without pulling in the daemon's main.cpp.
add_library(wowcapd-log STATIC
    LogLine.h
    LogLine.cpp
    LogWatcher.h
    LogWatcher.cpp
)

target_include_directories(wowcapd-log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(wowcapd-log PUBLIC Qt6::Core)
