add_library(constellar-client STATIC)
add_library(constellar::client ALIAS constellar-client)

find_program(QDBUSXML2CPP
  NAMES qdbusxml2cpp-qt6 qdbusxml2cpp6 qdbusxml2cpp
  REQUIRED
)

set(OBSERVER_XML ${CMAKE_SOURCE_DIR}/data/dev.ulduar.Constellar1.xml)
set(OBSERVER_PROXY_H ${CMAKE_CURRENT_BINARY_DIR}/observerproxy.h)
set(OBSERVER_PROXY_CPP ${CMAKE_CURRENT_BINARY_DIR}/observerproxy.cpp)

add_custom_command(
    OUTPUT ${OBSERVER_PROXY_H} ${OBSERVER_PROXY_CPP}
    COMMAND ${QDBUSXML2CPP} -p ${OBSERVER_PROXY_H}:${OBSERVER_PROXY_CPP} -c ObserverProxy
            ${OBSERVER_XML}
    DEPENDS ${OBSERVER_XML}
    COMMENT "Generating DBus ObserverProxy from ${OBSERVER_XML}"
    VERBATIM
  )

target_sources(constellar-client
  PRIVATE
    ${OBSERVER_PROXY_CPP}
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_BINARY_DIR}
    FILES
      ${OBSERVER_PROXY_H}
)

target_link_libraries(constellar-client
  PUBLIC
    Qt6::Core
    Qt6::DBus
)
