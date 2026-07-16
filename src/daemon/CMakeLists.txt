add_subdirectory(log)
add_subdirectory(controller)
add_subdirectory(presence)

add_executable(wowcapd
    main.cpp
    ManagerAdaptor.h
    ManagerAdaptor.cpp
    ManagerService.h
    ManagerService.cpp
)

target_link_libraries(wowcapd PRIVATE Qt6::Core Qt6::DBus wowcapd-common wowcapd-log
                                       wowcapd-controller wowcapd-presence)

install(TARGETS wowcapd)
