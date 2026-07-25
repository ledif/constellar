constellar_add_test(observerservice_test
    SOURCES ObserverServiceTest.h ObserverServiceTest.cpp
    LIBS constellar::observer
)

constellar_add_test(activityprojection_test
    SOURCES ActivityProjectionTest.h ActivityProjectionTest.cpp
    LIBS constellar::observer constellar::activitymetadata
)

constellar_add_test(interfaceregistry_test
    SOURCES InterfaceRegistryTest.h InterfaceRegistryTest.cpp
    LIBS constellar::observer
)

constellar_add_test(propertytreeobject_test
    SOURCES PropertyTreeObjectTest.h PropertyTreeObjectTest.cpp
    LIBS constellar::observer
)

constellar_add_test(objecttreepublisher_test DBUS_SESSION
    SOURCES ObjectTreePublisherTest.h ObjectTreePublisherTest.cpp
    LIBS constellar::observer Qt6::DBus
)
