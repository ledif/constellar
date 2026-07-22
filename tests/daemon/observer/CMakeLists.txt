constellar_add_test(observerservice_test
    SOURCES ObserverServiceTest.h ObserverServiceTest.cpp
    LIBS constellar::observer
)

constellar_add_test(activityprojection_test
    SOURCES ActivityProjectionTest.h ActivityProjectionTest.cpp
    LIBS constellar::observer constellar::activitymetadata
)

constellar_add_test(objectmanagerhost_test DBUS_SESSION
    SOURCES ObjectManagerHostTest.h ObjectManagerHostTest.cpp
    LIBS constellar::observer Qt6::DBus
)
