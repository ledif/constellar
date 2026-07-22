# DBUS_SESSION: the test talks to a real QDBusConnection::sessionBus(), so run it
# under dbus-run-session (its own private bus, same pattern as integration_status)
# instead of whatever session bus (if any) the calling shell happens to have.
function(constellar_add_test name)
    cmake_parse_arguments(ARG "DBUS_SESSION" "" "SOURCES;LIBS" ${ARGN})

    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE Qt6::Core Qt6::Test ${ARG_LIBS})

    if(ARG_DBUS_SESSION)
        add_test(NAME ${name} COMMAND dbus-run-session -- $<TARGET_FILE:${name}>)
    else()
        add_test(NAME ${name} COMMAND ${name})
    endif()
endfunction()

add_library(constellar_test_helpers INTERFACE)
add_library(constellar::test_helpers ALIAS constellar_test_helpers)
target_include_directories(constellar_test_helpers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/helpers)

add_library(constellar_test_fixtures INTERFACE)
add_library(constellar::test_fixtures ALIAS constellar_test_fixtures)
target_compile_definitions(constellar_test_fixtures
    INTERFACE LOGFIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

add_subdirectory(common)
add_subdirectory(daemon)
if(CONSTELLAR_BUILD_GUI)
    add_subdirectory(gui)
endif()
add_subdirectory(integration)
