# Discord Rich Presence publisher (RFC-002). Own static library, same
# reasoning as wowcapd-log/wowcapd-controller: tests/ link it without
# pulling in main.cpp.
add_library(wowcapd-presence STATIC
    DiscordIpcClient.h
    DiscordIpcClient.cpp
    PresencePublisher.h
    PresencePublisher.cpp
)

target_include_directories(wowcapd-presence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(wowcapd-presence PUBLIC Qt6::Core Qt6::Network)
