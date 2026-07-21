add_library(constellar-gamestate STATIC)
add_library(constellar::gamestate ALIAS constellar-gamestate)

target_sources(
  constellar-gamestate
    PRIVATE
      GameState.cpp
    PUBLIC
      FILE_SET HEADERS
      FILES
      GameState.h
)

target_link_libraries(constellar-gamestate PUBLIC Qt6::Core constellar::common)
