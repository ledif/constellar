add_executable(constellar-gui main.cpp)

target_link_libraries(constellar-gui PRIVATE Qt6::Core Qt6::DBus Qt6::Widgets constellar::common)

install(TARGETS constellar-gui)
