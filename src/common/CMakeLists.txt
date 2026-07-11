# Shared code: DBus constants + a generated client proxy for the Manager
# interface, consumed by both the CLI and the GUI so they never hand-roll
# their own marshalling.

find_program(QDBUSXML2CPP NAMES qdbusxml2cpp-qt6 qdbusxml2cpp6 qdbusxml2cpp REQUIRED)

set(MANAGER_XML ${CMAKE_SOURCE_DIR}/data/io.github.ledif.wowcapd.xml)
set(MANAGER_PROXY_H ${CMAKE_CURRENT_BINARY_DIR}/managerproxy.h)
set(MANAGER_PROXY_CPP ${CMAKE_CURRENT_BINARY_DIR}/managerproxy.cpp)

# Client-side proxy is fully mechanical, so it's generated. The server-side
# adaptor (src/daemon/ManagerAdaptor.*) is hand-written instead: the
# generated adaptor skeleton needs its method bodies filled in by hand
# anyway, so we skip generation there and keep data/io.github.ledif.wowcapd.xml
# as the documented source of truth for both sides.
add_custom_command(
    OUTPUT ${MANAGER_PROXY_H} ${MANAGER_PROXY_CPP}
    COMMAND ${QDBUSXML2CPP} -p ${MANAGER_PROXY_H}:${MANAGER_PROXY_CPP} -c ManagerProxy ${MANAGER_XML}
    DEPENDS ${MANAGER_XML}
    COMMENT "Generating DBus ManagerProxy from ${MANAGER_XML}"
    VERBATIM
)

add_library(wowcapd-common STATIC
    ${MANAGER_PROXY_H}
    ${MANAGER_PROXY_CPP}
)

target_include_directories(wowcapd-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(wowcapd-common PUBLIC Qt6::Core Qt6::DBus)
