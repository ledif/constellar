add_subdirectory(log)
add_subdirectory(controller)
add_subdirectory(presence)

add_executable(constellard
    main.cpp
    ObserverAdaptor.h
    ObserverAdaptor.cpp
    ObserverService.h
    ObserverService.cpp
)

target_link_libraries(constellard PRIVATE Qt6::Core Qt6::DBus constellar-common constellar-log
                                          constellar-controller constellar-presence)

install(TARGETS constellard)
