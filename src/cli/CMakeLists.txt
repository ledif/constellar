add_executable(wowcap main.cpp)

target_link_libraries(wowcap PRIVATE Qt6::Core Qt6::DBus wowcapd-common)

install(TARGETS wowcap)
