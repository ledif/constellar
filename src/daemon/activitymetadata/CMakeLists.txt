add_library(constellar-activitymetadata STATIC)
add_library(constellar::activitymetadata ALIAS constellar-activitymetadata)

target_sources(constellar-activitymetadata
  PRIVATE
    ActivityMetadata.cpp
  PUBLIC
    FILE_SET HEADERS
    FILES
    ActivityMetadata.h
)

target_link_libraries(constellar-activitymetadata
  PUBLIC
    Qt6::Core
    constellar::common
    constellar::tracker
)
