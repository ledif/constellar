add_executable(constellar main.cpp)

target_link_libraries(constellar PRIVATE Qt6::Core Qt6::DBus constellar-common)

install(TARGETS constellar)
