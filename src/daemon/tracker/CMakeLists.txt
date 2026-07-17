# Raids-only encounter state machine (PLAN.md §3.4). Its own static library,
# same reasoning as constellar-log: tests/ link it without pulling in main.cpp.
add_library(constellar-tracker STATIC
    ActivityTracker.h
    ActivityTracker.cpp
)

target_include_directories(constellar-tracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(constellar-tracker PUBLIC Qt6::Core constellar-log)
