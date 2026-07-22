add_library(constellar-presence STATIC)
add_library(constellar::presence ALIAS constellar-presence)

target_sources(constellar-presence
  PUBLIC
    FILE_SET HEADERS
    FILES
      DiscordIpcClient.h
      PresencePublisher.h
  PRIVATE
    DiscordIpcClient.cpp
    PresencePublisher.cpp
)

target_link_libraries(constellar-presence
  PUBLIC
    Qt6::Core
    Qt6::Network
    constellar::common
    constellar::gamestate
  PRIVATE
    constellar::contract
)
