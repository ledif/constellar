add_library(constellar-common STATIC)
add_library(constellar::common ALIAS constellar-common)

target_sources(constellar-common
  PRIVATE
    Activity.cpp
    Location.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
      Activity.h
      Location.h
)

target_link_libraries(constellar-common
  PUBLIC
    Qt6::Core
    Qt6::DBus
    constellar::contract
)
