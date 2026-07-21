constellar_add_test(presencepublisher_test
    SOURCES PresencePublisherTest.h PresencePublisherTest.cpp
    LIBS Qt6::Network constellar::presence constellar::test_helpers
)

constellar_add_test(discordipcclient_test
    SOURCES DiscordIpcClientTest.h DiscordIpcClientTest.cpp
    LIBS Qt6::Network constellar::presence constellar::test_helpers
)
