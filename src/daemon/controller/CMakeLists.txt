# Raids-only encounter state machine (PLAN.md §3.4). Its own static library,
# same reasoning as wowcapd-log: tests/ link it without pulling in main.cpp.
add_library(wowcapd-controller STATIC
    RecordingController.h
    RecordingController.cpp
)

target_include_directories(wowcapd-controller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(wowcapd-controller PUBLIC Qt6::Core wowcapd-log)
