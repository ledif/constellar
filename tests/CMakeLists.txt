function(constellar_add_test name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBS" ${ARGN})

    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE Qt6::Core Qt6::Test ${ARG_LIBS})

    add_test(NAME ${name} COMMAND ${name})
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
add_subdirectory(gui)
add_subdirectory(integration)
