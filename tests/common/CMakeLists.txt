constellar_add_test(activity_test
    SOURCES ActivityTest.h ActivityTest.cpp
    LIBS constellar::common
)

constellar_add_test(location_test
    SOURCES LocationTest.h LocationTest.cpp
    LIBS constellar::common
)
