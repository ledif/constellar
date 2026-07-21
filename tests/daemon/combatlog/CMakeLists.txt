constellar_add_test(logline_test
    SOURCES LogLineTest.h LogLineTest.cpp
    LIBS constellar::combatlog constellar::test_fixtures
)

constellar_add_test(logwatcher_test
    SOURCES LogWatcherTest.h LogWatcherTest.cpp
    LIBS constellar::combatlog
)

constellar_add_test(inotifyeventparser_test
    SOURCES InotifyEventParserTest.h InotifyEventParserTest.cpp
    LIBS constellar::combatlog
)
