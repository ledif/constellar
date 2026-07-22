# The wire contract vocabulary: the generated a{sv} key names (ActivityKeys.h,
# `just spec-gen` from data/dev.ulduar.Constellar1.spec.yaml) and the D-Bus
# service/interface/object-path constants (DBusConstants.h). Header-only, no Qt
# link needed. Kept in its own directory so consumers get *only* these headers on
# their include path, not the whole domain source tree.
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
