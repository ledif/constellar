find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(wowcapd-gui main.cpp)

target_link_libraries(wowcapd-gui PRIVATE Qt6::Core Qt6::DBus Qt6::Widgets wowcapd-common)

install(TARGETS wowcapd-gui)
