add_executable(logline_test LogLineTest.h LogLineTest.cpp)
target_link_libraries(logline_test PRIVATE Qt6::Core Qt6::Test constellar-log)
target_compile_definitions(logline_test PRIVATE LOGFIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/logfixtures")
add_test(NAME logline_test COMMAND logline_test)

add_executable(logwatcher_test LogWatcherTest.h LogWatcherTest.cpp)
target_link_libraries(logwatcher_test PRIVATE Qt6::Core Qt6::Test constellar-log)
add_test(NAME logwatcher_test COMMAND logwatcher_test)

add_executable(activitytracker_test ActivityTrackerTest.h ActivityTrackerTest.cpp)
target_link_libraries(activitytracker_test PRIVATE Qt6::Core Qt6::Test constellar-tracker)
add_test(NAME activitytracker_test COMMAND activitytracker_test)

add_executable(presencepublisher_test PresencePublisherTest.h PresencePublisherTest.cpp)
target_link_libraries(presencepublisher_test PRIVATE Qt6::Core Qt6::Test constellar-presence)
add_test(NAME presencepublisher_test COMMAND presencepublisher_test)

add_executable(gamestate_test GameStateTest.h GameStateTest.cpp)
target_link_libraries(gamestate_test PRIVATE Qt6::Core Qt6::Test constellar-gamestate)
add_test(NAME gamestate_test COMMAND gamestate_test)

add_executable(discordipcclient_test DiscordIpcClientTest.h DiscordIpcClientTest.cpp)
target_link_libraries(discordipcclient_test PRIVATE Qt6::Core Qt6::Network Qt6::Test constellar-presence)
add_test(NAME discordipcclient_test COMMAND discordipcclient_test)

add_executable(activity_test ActivityTest.h ActivityTest.cpp)
target_link_libraries(activity_test PRIVATE Qt6::Core Qt6::Test constellar-common)
add_test(NAME activity_test COMMAND activity_test)

add_executable(zone_test ZoneTest.h ZoneTest.cpp)
target_link_libraries(zone_test PRIVATE Qt6::Core Qt6::Test constellar-common)
add_test(NAME zone_test COMMAND zone_test)

add_executable(activitymetadatabuilder_test
    ActivityMetadataBuilderTest.h
    ActivityMetadataBuilderTest.cpp
)
target_link_libraries(activitymetadatabuilder_test PRIVATE Qt6::Core Qt6::Test
                                                            constellar-activitymetadata)
add_test(NAME activitymetadatabuilder_test COMMAND activitymetadatabuilder_test)
