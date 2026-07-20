function(constellar_add_test name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBS;COMPILE_DEFINITIONS" ${ARGN})

    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE Qt6::Core Qt6::Test ${ARG_LIBS})

    if(ARG_COMPILE_DEFINITIONS)
        target_compile_definitions(${name} PRIVATE ${ARG_COMPILE_DEFINITIONS})
    endif()

    add_test(NAME ${name} COMMAND ${name})
endfunction()

constellar_add_test(logline_test
    SOURCES LogLineTest.h LogLineTest.cpp
    LIBS constellar::combatlog
    COMPILE_DEFINITIONS LOGFIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

constellar_add_test(logwatcher_test
    SOURCES LogWatcherTest.h LogWatcherTest.cpp
    LIBS constellar::combatlog
)

constellar_add_test(inotifyeventparser_test
    SOURCES InotifyEventParserTest.h InotifyEventParserTest.cpp
    LIBS constellar::combatlog
)

constellar_add_test(activitytracker_test
    SOURCES ActivityTrackerTest.h ActivityTrackerTest.cpp
    LIBS constellar::tracker
)

constellar_add_test(presencepublisher_test
    SOURCES PresencePublisherTest.h PresencePublisherTest.cpp DiscordFrame.h
    LIBS Qt6::Network constellar::presence
)

constellar_add_test(gamestate_test
    SOURCES GameStateTest.h GameStateTest.cpp
    LIBS constellar::gamestate
)

constellar_add_test(discordipcclient_test
    SOURCES DiscordIpcClientTest.h DiscordIpcClientTest.cpp DiscordFrame.h
    LIBS Qt6::Network constellar::presence
)

constellar_add_test(activity_test
    SOURCES ActivityTest.h ActivityTest.cpp
    LIBS constellar::common
)

constellar_add_test(zone_test
    SOURCES ZoneTest.h ZoneTest.cpp
    LIBS constellar::common
)

constellar_add_test(activitymetadata_test
    SOURCES ActivityMetadataTest.h ActivityMetadataTest.cpp
    LIBS constellar::activitymetadata
)

constellar_add_test(observerservice_test
    SOURCES ObserverServiceTest.h ObserverServiceTest.cpp
    LIBS constellar::observer
)

constellar_add_test(gui_qml_load_test
    SOURCES GuiQmlLoadTest.h GuiQmlLoadTest.cpp
    LIBS Qt6::Gui Qt6::Qml Qt6::Quick constellar-qml
)
set_tests_properties(gui_qml_load_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

constellar_add_test(activitymodel_test
    SOURCES ActivityModelTest.h ActivityModelTest.cpp
    LIBS Qt6::Gui Qt6::Qml Qt6::Quick constellar-qml
)
set_tests_properties(activitymodel_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_test(NAME integration_status
    COMMAND dbus-run-session -- bash ${CMAKE_CURRENT_SOURCE_DIR}/integration/smoke.sh
            $<TARGET_FILE:constellard> $<TARGET_FILE:constellarctl>)
set_tests_properties(integration_status PROPERTIES LABELS integration TIMEOUT 30)
