add_executable(constellar main.cpp)

target_link_libraries(constellar PRIVATE Qt6::Core Qt6::DBus Qt6::Widgets constellar::common)

install(TARGETS constellar)
