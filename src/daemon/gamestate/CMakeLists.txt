add_library(constellar-gamestate STATIC GameState.h GameState.cpp)
add_library(constellar::gamestate ALIAS constellar-gamestate)

target_include_directories(constellar-gamestate PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                                       ${CMAKE_SOURCE_DIR}/src/common)

target_link_libraries(constellar-gamestate PUBLIC Qt6::Core)
