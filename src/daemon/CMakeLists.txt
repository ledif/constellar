add_executable(wowcapd
    main.cpp
    ManagerAdaptor.h
    ManagerAdaptor.cpp
    ManagerService.h
    ManagerService.cpp
)

target_link_libraries(wowcapd PRIVATE Qt6::Core Qt6::DBus wowcapd-common)

install(TARGETS wowcapd)
