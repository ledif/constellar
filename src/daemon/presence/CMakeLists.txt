# Discord Rich Presence publisher (RFC-002). Own static library, same
# reasoning as constellar-log/constellar-controller: tests/ link it without
# pulling in main.cpp.
add_library(constellar-presence STATIC
    DiscordIpcClient.h
    DiscordIpcClient.cpp
    PresencePublisher.h
    PresencePublisher.cpp
)

target_include_directories(constellar-presence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(constellar-presence PUBLIC Qt6::Core Qt6::Network)
