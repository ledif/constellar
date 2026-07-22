# The wire contract vocabulary
add_library(constellar-contract INTERFACE)
add_library(constellar::contract ALIAS constellar-contract)

target_sources(constellar-contract
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
      ActivityKeys.h
      DBusConstants.h
)
