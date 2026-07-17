find_program(QDBUSXML2CPP NAMES qdbusxml2cpp-qt6 qdbusxml2cpp6 qdbusxml2cpp REQUIRED)

set(OBSERVER_XML ${CMAKE_SOURCE_DIR}/data/io.github.ledif.constellar.xml)
set(OBSERVER_PROXY_H ${CMAKE_CURRENT_BINARY_DIR}/observerproxy.h)
set(OBSERVER_PROXY_CPP ${CMAKE_CURRENT_BINARY_DIR}/observerproxy.cpp)

add_custom_command(
    OUTPUT ${OBSERVER_PROXY_H} ${OBSERVER_PROXY_CPP}
    COMMAND ${QDBUSXML2CPP} -p ${OBSERVER_PROXY_H}:${OBSERVER_PROXY_CPP} -c ObserverProxy ${OBSERVER_XML}
    DEPENDS ${OBSERVER_XML}
    COMMENT "Generating DBus ObserverProxy from ${OBSERVER_XML}"
    VERBATIM
)

add_library(constellar-common STATIC
    ${OBSERVER_PROXY_H}
    ${OBSERVER_PROXY_CPP}
    Activity.h
    Activity.cpp
    Zone.h
    Zone.cpp
)

target_include_directories(constellar-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(constellar-common PUBLIC Qt6::Core Qt6::DBus)
