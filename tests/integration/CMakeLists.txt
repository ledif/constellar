add_test(NAME integration_status
    COMMAND dbus-run-session -- bash ${CMAKE_CURRENT_SOURCE_DIR}/smoke.sh
            $<TARGET_FILE:constellard> $<TARGET_FILE:constellarctl>
)

set_tests_properties(integration_status PROPERTIES
     LABELS integration
     TIMEOUT 30
)
