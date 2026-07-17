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
add_subdirectory(tracker)
add_subdirectory(presence)

# Translates ActivityTracker's decision types into GameState's QVariantMap
# metadata dictionaries (ADR-012). Own static library so tests/ can exercise
# the translation without constructing the whole pipeline.
add_library(constellar-activitymetadata STATIC
    ActivityMetadataBuilder.h
    ActivityMetadataBuilder.cpp
)

target_include_directories(constellar-activitymetadata PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(constellar-activitymetadata PUBLIC Qt6::Core constellar-common
                                                          constellar-tracker)

add_executable(constellard
    main.cpp
    ObserverAdaptor.h
    ObserverAdaptor.cpp
    ObserverService.h
    ObserverService.cpp
)

target_link_libraries(constellard PRIVATE Qt6::Core Qt6::DBus constellar-common constellar-log
                                          constellar-tracker constellar-activitymetadata
                                          constellar-presence constellar-gamestate)

install(TARGETS constellard)
