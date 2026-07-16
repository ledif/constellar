add_executable(logline_test LogLineTest.h LogLineTest.cpp)
target_link_libraries(logline_test PRIVATE Qt6::Core Qt6::Test wowcapd-log)
target_compile_definitions(logline_test PRIVATE LOGFIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/logfixtures")
add_test(NAME logline_test COMMAND logline_test)

add_executable(logwatcher_test LogWatcherTest.h LogWatcherTest.cpp)
target_link_libraries(logwatcher_test PRIVATE Qt6::Core Qt6::Test wowcapd-log)
add_test(NAME logwatcher_test COMMAND logwatcher_test)

add_executable(recordingcontroller_test RecordingControllerTest.h RecordingControllerTest.cpp)
target_link_libraries(recordingcontroller_test PRIVATE Qt6::Core Qt6::Test wowcapd-controller)
add_test(NAME recordingcontroller_test COMMAND recordingcontroller_test)

add_executable(presencepublisher_test PresencePublisherTest.h PresencePublisherTest.cpp)
target_link_libraries(presencepublisher_test PRIVATE Qt6::Core Qt6::Test wowcapd-presence)
add_test(NAME presencepublisher_test COMMAND presencepublisher_test)

add_executable(discordipcclient_test DiscordIpcClientTest.h DiscordIpcClientTest.cpp)
target_link_libraries(discordipcclient_test PRIVATE Qt6::Core Qt6::Network Qt6::Test wowcapd-presence)
add_test(NAME discordipcclient_test COMMAND discordipcclient_test)
