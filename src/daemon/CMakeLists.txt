# GameState is the shared fact-bag state (ADR-012) that both ObserverService
# and PresencePublisher (constellar-presence) need, so it's its own static
# library rather than living inline in the constellard executable target --
# same reasoning as log/controller/presence, just flat rather than in a
# subdirectory since it's a single header+source pair.
add_library(constellar-gamestate STATIC
    GameState.h
    GameState.cpp
)

target_include_directories(constellar-gamestate PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/common
)

target_link_libraries(constellar-gamestate PUBLIC Qt6::Core)

add_subdirectory(log)
add_subdirectory(controller)
add_subdirectory(presence)

add_executable(constellard
    main.cpp
    ObserverAdaptor.h
    ObserverAdaptor.cpp
    ObserverService.h
    ObserverService.cpp
)

target_link_libraries(constellard PRIVATE Qt6::Core Qt6::DBus constellar-common constellar-log
                                          constellar-controller constellar-presence
                                          constellar-gamestate)

install(TARGETS constellard)
